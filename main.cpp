
#include "ap.h"
#include "netcdf.h"
#include <algorithm>
#include <bits/stdc++.h>
#include <cctype>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <time.h>
#include <unistd.h>

using namespace std;

inline char to_uppercase(unsigned char c);
void handle_error(int status, int Paso);
inline void ltrim(std::string &s);
inline void rtrim(std::string &s);
clock_t Timestamp();
string Ruta, NomVar;
double LBound = -1, RBound = -1;
int Ntimes = 3, TipoSp = -1;
int var_id, var_ndims, var_dimids[NC_MAX_VAR_DIMS], var_natts;
nc_type var_type;
int status, ncid;

int main(int, char **) {
  std::cout << "Hello, from test!\n";

  try {
    ifstream config("DiffMotion.cfig");
    string cadena;
    int Iterador = 0;

    if (config.is_open()) {
      while (getline(config, cadena)) {
        if (Iterador == 0) {
          Ruta = cadena;
        } else if (Iterador == 1) {
          Ntimes = stoi(cadena);
        } else if (Iterador == 2) {
          NomVar = cadena;
        } else if (Iterador == 3) {
          transform(cadena.cbegin(), cadena.cend(), cadena.begin(),
                    to_uppercase);
          if (cadena == "NATURAL") {
            TipoSp = 1;
          } else if (cadena == "PARABOLIC") {
            TipoSp = 2;
          } else if (cadena == "CLAMPED") {
            TipoSp = 3;
          }
          if (TipoSp == -1)
            throw "err: No hay un tipo válido de spline.";
        } else if (Iterador == 4 && TipoSp == 3) {
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
    char ruta[Ruta.length() + 1];
    strcpy(ruta, Ruta.c_str());
    char nomvar[NomVar.length() + 1];
    strcpy(nomvar, NomVar.c_str());

    status = nc_open(ruta, 0, &ncid);
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
    if (var_type == NC_FLOAT) {
    } else if (var_type == NC_DOUBLE) {
    } else
      throw "Variable de tipo no soportado.";

    string Nombres[var_ndims];
    size_t Tamanhos[var_ndims];

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
    /*
    const size_t NDIM = 3;
    float Var_valoresf[Tamanhos[1] * Tamanhos[2] * Tamanhos[0]];
    double Var_valoresd[Tamanhos[1] * Tamanhos[2] * Tamanhos[0]];
    static size_t startp[NDIM];
    static size_t countp[NDIM];
    static ptrdiff_t stridep[NDIM];
    static ptrdiff_t imapp[NDIM];

    for (int i = 0; i < 3; i++) {
      startp[i] = 0;
      stridep[i] = 1;
      countp[i] = Tamanhos[i];
    }
    imapp[0] = Tamanhos[1];
    imapp[1] = 1;
    imapp[2] = Tamanhos[0];

    if (var_type == NC_FLOAT) {
      nc_get_varm_float(ncid, var_id, startp, countp, stridep, imapp,
                        Var_valoresf);
      if (status != NC_NOERR)
        handle_error(status, 5);
    } else if (var_type == NC_DOUBLE) {
      nc_get_varm_double(ncid, var_id, startp, countp, stridep, imapp,
                         Var_valoresd);
      if (status != NC_NOERR)
        handle_error(status, 5);
    }
    */
  } catch (exception e) {
    exit(EXIT_FAILURE);
  }
  // Source - https://stackoverflow.com/a/42544
  // Posted by 1800 INFORMATION, modified by community. See post 'Timeline' for
  // change history Retrieved 2026-08-07, License - CC BY-SA 3.0
  id_t c_pid = fork();
  if (c_pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  } else if (c_pid > 0) {
    //  wait(nullptr);
    cout << "printed from parent process " << getpid() << endl;
    throw "err: No se pudo iniciar el proceso de NCO.";
  } else {
    char *Argums[3] = {(char *)"-a"};
    cout << "printed from child process " << getpid() << endl;
    execve("ncpdq", Argums, NULL);
  }
  const int Duracion = 2;
  clock_t TActual = Timestamp();
  clock_t Final = TActual + Duracion * (double)CLOCKS_PER_SEC;
  int Estado;

  kill(getpid(), 15); // SIGTERM
  while (TActual < Final) {
    Estado = kill(getpid(), -0);
    if (Estado == 0)
      break;
    // Source - https://stackoverflow.com/a/72274580
    // Posted by eerorika, modified by community. See post 'Timeline' for change
    // history Retrieved 2026-08-07, License - CC BY-SA 4.01    
    this_thread::sleep_for(chrono::milliseconds(100));
    TActual = Timestamp();
  }
  if (TActual <= Final)
    kill(getpid(), 9); // SIGKILL

  // ncpdq - a lon, lev, lat - v three_dmn_var in.nc out.nc
  // alglib::real_1d_array bb; bb.setcontent(ae_int_t iLen, const double
  // *pContent); alglib::spline1dconvdiffcubic());

  status = nc_close(ncid);
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
              .base(),
          s.end());
}

clock_t Timestamp() {
  struct timespec tw1; // both C11 and POSIX
  // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts1); // POSIX
  clock_gettime(CLOCK_MONOTONIC, &tw1); // POSIX; use timespec_get in C11
  clock_t t1 = clock();
  return t1;
  ;
}
