
#include "ap.h"
#include "interpolation.h"
#include "netcdf.h"
#include <algorithm>
#include <bits/stdc++.h>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ostream>
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
template <class tipo> void findMinMax(tipo arr[], int n, tipo *max, tipo *min);

int main(int, char **) {
  // std::cout << "Hello, from test!\n";
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

    // Obtención de los datos del fichero de configuración.
    ifstream config("DiffMotion.cfig");
    string cadena = string(255, ' ');
    int Iterador = 0;
    if (config.is_open()) {
      while (getline(config, cadena)) {
        rtrim(cadena);
        ltrim(cadena);
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
        } else if (Iterador == 5 && (TipoSp == 3 | TipoSp == 4)) {
          stringstream Cadena(cadena);
          string token = string(255, ' ');
          int Iterador_2 = 0;
          while (getline(Cadena, token, ',')) {
            rtrim(cadena);
            ltrim(cadena);
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
    config.close();
    // Fin de Obtención de los datos del fichero de configuración.

    // Generar los nombres de ficheros con ruta en string y char * .
    if (Ruta.back() != '/')
      Ruta += "/";
    string InFich = Ruta + NomFich;
    size_t Punto = NomFich.find_last_of('.');
    string NomFich2 = NomFich.insert(Punto, "_Dif");
    string OutFich = Ruta + NomFich2;
    if (filesystem::exists(NomFich2))
      throw "err: El fichero destino ya existe";
    filesystem::copy(InFich, OutFich);
    const char *inruta = InFich.c_str();
    const char *outruta = OutFich.c_str();
    const char *nomvar = NomVar.c_str();
    // Fin de Generar los nombres de ficheros con ruta en string y char * .

    // Obtener acceso al fichero de entrada, la variable y sus dimensiones.
    status = nc_open(inruta, NC_NOWRITE, &ncid);
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
    if (Ntimes < Tamanhos[0] && Ntimes > 2)
      Tamanho = Ntimes;
    else
      Tamanho = Tamanhos[0];
    // Fin de Obtener acceso al fichero den entrada, la variable y sus
    // dimensiones.

    // Bucle principal de iteración por dimensiones.
    double Datas[Tamanho], Abscisas[Tamanho];
    double G_vr_val[2];
    bool chivato[3] = {false, false, false}, roto = false;
    size_t NReg_Tratados = 0, NReg_excluidos = 0;
    const size_t NReg_total = Tamanhos[0] * Tamanhos[1] * Tamanhos[2];
    double max, min, Max, Min;

    // Se crea el fichero de salida.
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

          // Se realizan las mismas acciones para tipos FLOAT, DOUBLE e INT
          if (var_type == NC_FLOAT) {

            float vr_val[2], fill_val;
            // Operación a realizar una sola vez al comienzo.
            if (!chivato[0]) {
              status = nc_get_att_float(ncid, var_id, "valid_range", vr_val);
              if (status != NC_NOERR)
                handle_error(status, 6);
              status = nc_get_att_float(ncid, var_id, "_FillValue", &fill_val);
              if (status != NC_NOERR)
                handle_error(status, 7);
              for (int v = 0; v < 2; v++) {
                G_vr_val[v] = vr_val[v];
              }
              chivato[0] = true;
            }
            status = nc_get_var1_float(ncid, var_id, var_index, &data_val_f);
            if (status != NC_NOERR)
              handle_error(status, 12);
            if (isnan(data_val_f) || data_val_f == fill_val) {
              roto = true;
              NReg_excluidos++;
              break;
            }
            Datas[n] = data_val_f;
            NReg_Tratados++;

          } else if (var_type == NC_DOUBLE) {

            double vr_val[2], fill_val;
            // Operación a realizar una sola vez al comienzo.
            if (!chivato[0]) {
              status = nc_get_att_double(ncid, var_id, "valid_range", vr_val);
              if (status != NC_NOERR)
                handle_error(status, 8);
              status = nc_get_att_double(ncid, var_id, "_FillValue", &fill_val);
              if (status != NC_NOERR)
                handle_error(status, 9);
              for (int v = 0; v < 2; v++) {
                G_vr_val[v] = vr_val[v];
              }
              chivato[0] = true;
            }
            status = nc_get_var1_double(ncid, var_id, var_index, &data_val_d);
            if (status != NC_NOERR)
              handle_error(status, 12);
            if (isnan(data_val_d) || data_val_d == fill_val) {
              NReg_excluidos++;
              roto = true;
              break;
            }
            Datas[n] = data_val_d;
            NReg_Tratados++;

          } else if (var_type == NC_INT) {

            int vr_val[2], fill_val;
            // Operación a realizar una sola vez al comienzo.
            if (!chivato[0]) {
              status = nc_get_att_int(ncid, var_id, "valid_range", vr_val);
              if (status != NC_NOERR)
                handle_error(status, 10);
              status = nc_get_att_int(ncid, var_id, "_FillValue", &fill_val);
              if (status != NC_NOERR)
                handle_error(status, 11);
              for (int v = 0; v < 2; v++) {
                G_vr_val[v] = vr_val[v];
              }
              chivato[0] = true;
            }
            status = nc_get_var1_int(ncid, var_id, var_index, &data_val_i);
            if (status != NC_NOERR)
              handle_error(status, 12);
            if (isnan(data_val_i) || data_val_i == fill_val) {
              NReg_excluidos++;
              roto = true;
              break;
            }
            Datas[n] = data_val_i;
            NReg_Tratados++;
          }
        }
        // Si ha salido del bucle por break, se le hace continuar.
        if (roto) {
          roto = false;
          continue;
        }
        // Los valores almacenados en Datas y Abscisas son los valores de
        // entrada de la función de interpolación.
        alglib::real_1d_array data;
        data.setcontent(Tamanho, Datas);
        alglib::real_1d_array abscisas;
        abscisas.setcontent(Tamanho, Abscisas);
        alglib::real_1d_array yes, dyes;

        if (TipoSp == 1) { // Natural
          alglib::spline1dconvdiffcubic(abscisas, data, Tamanho, 2, 0, 2, 0,
                                        abscisas, Tamanho, yes, dyes);
        }
        if (TipoSp == 2) { // Parabólica
          alglib::spline1dconvdiffcubic(abscisas, data, Tamanho, 0, 0, 0, 0,
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
        /*
        double unarray[Tamanho];
        double *puntero;
        puntero = dyes.getcontent();
        for (int bb = 0; bb < Tamanho; bb++) {
          unarray[bb] = *puntero;
          puntero++;
        }
        */
        // Se almacena el máximo y el mínimo para poder variar luego
        // el atributo netcdf "valid-range" .
        findMinMax<double>(dyes.getcontent(), Tamanho, &max, &min);
        if (!chivato[1]) {
          Min = min;
          Max = max;
          chivato[1] = true;
        } else {
          if (max > Max)
            Max = max;
          if (min < Min)
            Min = min;
        }
        // Se itera de nuevo para inroducirlo en el nuevo fichero.
        for (int n = 0; n < Tamanho; n++) {
          var_index[0] = n;

          // Se realizan las mismas acciones para tipos FLOAT, DOUBLE e INT
          if (var_type == NC_FLOAT) {
            float vr_val[2];
            // Esta operación de realiza una vez al comienzo.
            if (!chivato[2]) {
              vr_val[0] = MAXFLOAT;
              vr_val[1] = -MAXFLOAT;
              status = nc_redef(ncid_out);
              if (status != NC_NOERR)
                handle_error(status, 13);

              status = nc_put_att_float(ncid_out, var_id, "valid_max", NC_FLOAT,
                                        1, &vr_val[0]);
              if (status != NC_NOERR)
                handle_error(status, 14);
              status = nc_put_att_float(ncid_out, var_id, "valid_min", NC_FLOAT,
                                        1, &vr_val[1]);
              if (status != NC_NOERR)
                handle_error(status, 15);
              status = nc_enddef(ncid_out); /*leave define mode*/
              if (status != NC_NOERR)
                handle_error(status, 15);
              chivato[2] = true;
            }
            data_val_f = (float)dyes.getcontent()[n];
            status =
                nc_put_var1_float(ncid_out, var_id, var_index, &data_val_f);

          } else if (var_type == NC_DOUBLE) {

            double vr_val[2];
            // Esta operación de realiza una vez al comienzo.
            if (!chivato[2]) {
              vr_val[0] = Max * 1.25;
              vr_val[1] = Min * 0.75;
              status = nc_redef(ncid_out);
              if (status != NC_NOERR)
                handle_error(status, 16);
              status = nc_put_att_double(ncid_out, var_id, "valid_range",
                                         NC_DOUBLE, 2, vr_val);
              if (status != NC_NOERR)
                handle_error(status, 17);
              status = nc_enddef(ncid_out); /*leave define mode*/
              if (status != NC_NOERR)
                handle_error(status, 18);
              chivato[2] = true;
            }
            data_val_d = dyes.getcontent()[n];
            status =
                nc_put_var1_double(ncid_out, var_id, var_index, &data_val_d);

          } else if (var_type == NC_INT) {

            int vr_val[2];
            // Esta operación de realiza una vez al comienzo.
            if (!chivato[2]) {
              vr_val[0] = (int)Max * 1.25;
              vr_val[1] = (int)Min * 0.75;
              status = nc_redef(ncid_out);
              if (status != NC_NOERR)
                handle_error(status, 19);
              status = nc_put_att_int(ncid_out, var_id, "valid_range", NC_INT,
                                      2, vr_val);
              if (status != NC_NOERR)
                handle_error(status, 20);
              status = nc_enddef(ncid_out); /*leave define mode*/
              if (status != NC_NOERR)
                handle_error(status, 21);
              chivato[2] = true;
            }
            data_val_i = (int)dyes.getcontent()[n];
            status = nc_put_var1_int(ncid_out, var_id, var_index, &data_val_i);
          }
          if (status != NC_NOERR)
            handle_error(status, 22);
        }
      }
    }
    // Fin de Bucle principal de iteración por dimensiones.
    if (var_type == NC_FLOAT) {
      float vr_val[2];
      vr_val[0] = Max * 1.1f;
      vr_val[1] = Min * 0.9f;
      status = nc_redef(ncid_out);
      if (status != NC_NOERR)
        handle_error(status, 13);
      status = nc_put_att_float(ncid_out, var_id, "valid_max", NC_FLOAT, 1,
                                &vr_val[0]);
      if (status != NC_NOERR)
        handle_error(status, 14);
      status = nc_put_att_float(ncid_out, var_id, "valid_min", NC_FLOAT, 1,
                                &vr_val[1]);
      if (status != NC_NOERR)
        handle_error(status, 15);
      status = nc_enddef(ncid_out); /*leave define mode*/
      if (status != NC_NOERR)
        handle_error(status, 15);
    }    
    ofstream salida(Ruta + "info.out");
    if (!salida.is_open())
      throw "err: no se ha podido crear el fichero info de salida.";
    salida << "Iteración " << "[" << var_index[0] << "," << var_index[1] << ","
           << var_index[2] << "]" << endl;
    salida << "Total Registros: " << NReg_total << endl;
    salida << "Total Registros tratados: " << NReg_Tratados << endl;
    salida << "Total Registros excluidos: " << NReg_excluidos << endl << endl;
    salida << "Mínimo: " << Min << endl;
    salida << "Máximo: " << Max << endl;
    salida.close();
    status = nc_close(ncid_out);
    status = nc_close(ncid);

  } catch (exception e) {
    cout << e.what() << endl;
    exit(EXIT_FAILURE);
  }
}

inline char to_uppercase(unsigned char c) { return toupper(c); }

void handle_error(int status, int Paso) {}

// Source - https://stackoverflow.com/a/217605
// Posted by Evan Teran, modified by community. See post 'Timeline' for change
// history Retrieved 2026-08-07, License - CC BY-SA 4.0 Trim from the start
// (in place)
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

template <class tipo> void findMinMax(tipo arr[], int n, tipo *max, tipo *min) {
  *min = arr[0];
  *max = arr[0];
  for (int i = 1; i < n; i++) { // Actualiza el máximo si arr[i] es mayor
    if (arr[i] > *max)
      *max = arr[i]; // Actualiza el mínimo si arr[i] es menor
    if (arr[i] < *min)
      *min = arr[i];
  }
}