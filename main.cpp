
#include "ap.h"
#include "interpolation.h"
#include "netcdf.h"
#include <algorithm>
#include <bits/stdc++.h>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unistd.h>

using namespace std;

inline char to_uppercase(unsigned char c);
void handle_error(int status, int Paso);
inline void ltrim(std::string &s);
inline void rtrim(std::string &s);
clock_t Timestamp();

int main(int, char **) {
  //std::cout << "Hello, from test!\n";
  string Ruta, NomFich, NomVar;
  double LBound = -1, RBound = -1;
  int Ntimes = 3, TipoSp = -1;
  int var_id, var_ndims, var_dimids[NC_MAX_VAR_DIMS], var_natts;
  nc_type var_type;
  int status, ncid, ncid_out;
  string Nombres[3];
  size_t Tamanhos[3];
  double data_val_d;
  float data_val_f;
  int data_val_i;
  size_t var_index[3];

  try {
    ifstream config("DiffMotion.cfig");
    string cadena;
    int Iterador = 0;

    if (config.is_open()) {
      while (getline(config, cadena)) {
        if (Iterador == 0) {
          Ruta = cadena;
        } else if (Iterador == 1) {
          NomFich = cadena;
        } else if (Iterador == 2) {
          Ntimes = stoi(cadena);
        } else if (Iterador == 3) {
          NomVar = cadena;
        } else if (Iterador == 4) {
          transform(cadena.cbegin(), cadena.cend(), cadena.begin(),
                    to_uppercase);
          if (cadena == "NATURAL") {
            TipoSp = 1;
          } else if (cadena == "PARABOLIC") {
            TipoSp = 2;
          } else if (cadena == "CLAMPED_1D") {
            TipoSp = 3;
          } else if (cadena == "CLAMPED_2D") {
            TipoSp = 4;
          }
          if (TipoSp == -1)
            throw "err: No hay un tipo válido de spline.";
        } else if (Iterador == 5 && TipoSp == 3) {
          stringstream Cadena(cadena);
          string token;
          int Iterador_2 = 0;
          while (getline(Cadena, token, ',')) {
            if (Iterador_2 == 0)
              LBound = stod(token);
            else if (Iterador_2 == 1)
              RBound = stod(token);
            else
              break;
            Iterador_2++;
          }
        } else
          break;
        Iterador++;
      }
    }
    if (Ruta.back() != '/')
      Ruta += "/";
    string InFich = Ruta + NomFich;
    size_t Punto = NomFich.find_last_of('.');
    string NomFich2 = NomFich.insert(Punto - 1, "_Dif");    
    string OutFich = Ruta + NomFich2;
    filesystem::copy_file(InFich, OutFich);

    char inruta[InFich.length() + 1];
    strcpy(inruta, InFich.c_str());
    char outruta[OutFich.length() + 1];
    strcpy(outruta, OutFich.c_str());
    char nomvar[NomVar.length() + 1];
    strcpy(nomvar, NomVar.c_str());

    status = nc_open(inruta, 0, &ncid);
    if (status != NC_NOERR)
      handle_error(status, 1);
    status = nc_inq_varid(ncid, nomvar, &var_id);
    if (status != NC_NOERR)
      handle_error(status, 2);
    status = nc_inq_var(ncid, var_id, 0, &var_type, &var_ndims, var_dimids,
                        &var_natts);
    if (status != NC_NOERR)
      handle_error(status, 3);
    if (var_ndims != 3)
      throw "El numero de dimensiones es distinto de tres.";
    if (var_type != NC_FLOAT && var_type != NC_DOUBLE && var_type != NC_INT)
      throw "Variable de tipo no soportado.";

    for (int i = 0; i < var_ndims; i++) {
      size_t recs;
      char recname[NC_MAX_NAME + 1];
      status = nc_inq_dim(ncid, var_dimids[i], recname, &recs);
      if (status != NC_NOERR)
        handle_error(status, 4);
      Nombres[i] = recname;
      ltrim(Nombres[i]);
      rtrim(Nombres[i]);
      Tamanhos[i] = recs;
    }
    size_t Tamanho;
    if (Ntimes < Tamanhos[0])
      Tamanho = Ntimes;
    else
      Tamanho = Tamanhos[0];

    double Datas[Tamanho], Abscisas[Tamanho];

    status = nc_open(outruta, NC_WRITE, &ncid_out);
    if (status != NC_NOERR)
      handle_error(status, 5);

    for (int i = 0; i < Tamanhos[1]; i++) {
      var_index[1] = i;
      for (int m = 0; m < Tamanhos[2]; m++) {
        var_index[2] = m;
        for (int n = 0; n < Tamanho; n++) {
          Abscisas[n] = n;
          var_index[0] = n;
          if (var_type == NC_FLOAT) {
            status = nc_get_var1_float(ncid, var_id, var_index, &data_val_f);
            Datas[n] = data_val_f;
          } else if (var_type == NC_DOUBLE) {
            status = nc_get_var1_double(ncid, var_id, var_index, &data_val_d);
            Datas[n] = data_val_d;
          } else if (var_type == NC_INT) {
            status = nc_get_var1_int(ncid, var_id, var_index, &data_val_i);
            Datas[n] = data_val_i;
          }
          if (status != NC_NOERR)
            handle_error(status, 6);
        }
        alglib::real_1d_array data;
        data.setcontent(Tamanho, Datas);
        alglib::real_1d_array abscisas;
        abscisas.setcontent(Tamanho, Abscisas);
        alglib::real_1d_array yes, dyes;

        if (TipoSp == 1) { // Parabólica
          alglib::spline1dconvdiffcubic(abscisas, data, Tamanho, 0, 0, 0, 0,
                                        abscisas, Tamanho, yes, dyes);
        }
        if (TipoSp == 2) { // Natural
          alglib::spline1dconvdiffcubic(abscisas, data, Tamanho, 2, 0, 2, 0,
                                        abscisas, Tamanho, yes, dyes);
        }
        if (TipoSp == 3) { // Clamped1d
          alglib::spline1dconvdiffcubic(abscisas, data, Tamanho, 1, LBound, 1,
                                        RBound, abscisas, Tamanho, yes, dyes);
        }
        if (TipoSp == 4) { // Clamped2d
          alglib::spline1dconvdiffcubic(abscisas, data, Tamanho, 2, LBound, 2,
                                        RBound, abscisas, Tamanho, yes, dyes);
        }
        if (status != NC_NOERR)
          handle_error(status, 6);

        for (int n = 0; n < Tamanho; n++) {
          var_index[0] = n;
          data_val_d = dyes.getcontent()[n];
          data_val_f = (float)dyes.getcontent()[n];
          data_val_i = (int)dyes.getcontent()[n];

          if (var_type == NC_FLOAT) {
            status =
                nc_put_var1_float(ncid_out, var_id, var_index, &data_val_f);
          } else if (var_type == NC_DOUBLE) {
            status =
                nc_put_var1_double(ncid_out, var_id, var_index, &data_val_d);
          } else if (var_type == NC_INT) {
            status = nc_put_var1_int(ncid_out, var_id, var_index, &data_val_i);
          }
          if (status != NC_NOERR)
            handle_error(status, 6);
        }
      }
      status = nc_close(ncid_out);
      status = nc_close(ncid);
    }
  } catch (exception e) {
    exit(EXIT_FAILURE);
  }
}

inline char to_uppercase(unsigned char c) { return toupper(c); }

void handle_error(int status, int Paso) {}

// Source - https://stackoverflow.com/a/217605
// Posted by Evan Teran, modified by community. See post 'Timeline' for change
// history Retrieved 2026-08-07, License - CC BY-SA 4.0 Trim from the start (in
// place)
inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}
// Trim from the end (in place)
inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(), s.end());
}

clock_t Timestamp() {
  struct timespec tw1; // both C11 and POSIX
  // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts1); // POSIX
  clock_gettime(CLOCK_MONOTONIC, &tw1); // POSIX; use timespec_get in C11
  clock_t t1 = clock();
  return t1;
}
