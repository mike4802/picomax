#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


int read_line(int fd, char *line, int max)
{
    int i = 0;
    char c;

    while (i < max - 1) {
        int n = read(fd, &c, 1);

        if (n <= 0)
            return -1;

        if (c == '\n') {
            line[i] = '\0';
            return i;
        }

        line[i++] = c;
    }

    line[i] = '\0';
    return i;
}


int main() {
  int skiplines = 0;
  char line[128];
  unsigned int ms;
  unsigned int counts;

  int fd = open("/dev/ttyACM0", O_RDONLY);
  
  FILE *logfile = fopen("data.csv", "a");

  if (!logfile) {
      perror("fopen");
      return 1;
  }


  if (fd < 0)
  {
      perror("open");
      return 1;
  }

  fprintf(logfile, "timestamp,elapsed_ms,counts,temp_c,temp_f\n");

  while (1)
  {

        if (read_line(fd, line, sizeof(line)) > 0) {
            sscanf(line, "%u,%u", &ms, &counts);
            float temp_c = counts * 0.25f;
            float temp_f = temp_c * 9.0f / 5.0f + 32.0f;

            if (skiplines %10 == 0) {


                printf("C=%f F=%f\n", temp_c, temp_f);

                time_t now = time(NULL);
                fprintf(logfile, "%ld,%u,%u,%f,%f\n", now, ms, counts, temp_c, temp_f);
                fflush(logfile);

                char cmd[256];
                snprintf(cmd, sizeof(cmd), 
                    "rrdtool update office.rrd N:%.2f",temp_f);
                system(cmd);
            }

            skiplines++;
        }
  }
  close(fd);
  fclose(logfile);

  return 0;
}
