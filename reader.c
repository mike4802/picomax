#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>


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

  char line[128];
  unsigned int ms;
  unsigned int counts;

  int fd = open("/dev/cu.usbmodem11401", O_RDONLY);

  if (fd < 0)
  {
      perror("open");
      return 1;
  }

  while (1)
  {
    if (read_line(fd, line, sizeof(line)) > 0) {
      sscanf(line, "%u,%u", &ms, &counts);
      float temp_c = counts * 0.25f;
      float temp_f = temp_c * 9.0f / 5.0f + 32.0f;
      printf("C=%f F=%f\n", temp_c, temp_f);
    }
  }
    close(fd);

    return 0;
}
