#include <stdio.h>

struct ioif {
  int (*wmem) (unsigned int addr, const unsigned int *data, int len);
  int (*gmem) (unsigned int addr, unsigned int *data, int len);
  int (*open) (char *device, int baudrate, int port);
  int (*close) ();
  int (*setbaud) (int baud, int pp);
  int (*init) (char* arg);
};

static int my_wmem (unsigned int addr, const unsigned int *data, int len)
{
  printf("my_wmem: addr = %x, *data = %p, len = %d\n",
  	addr, data, len);
  return len;
}

static int my_gmem (unsigned int addr, unsigned int *data, int len)
{
  printf("my_gmem: addr = %x, *data = %p, len = %d\n",
  	addr, data, len);
  return len;
}

static int my_open(char *device, int baudrate, int port)
{
   return 0;
}

static int my_close()
{
   printf("my_close:\n");
   return 0;
}

static int my_setbaud (int baud, int pp)
{
  printf("my_setbaud: baud = %d, pp = %d\n", baud, pp);
  return 0;
}

static int my_init (char* arg)
{
   printf("my_init: arg = %s\n", arg);
   return 0; // Success
}

static struct ioif my_io = 
   {my_wmem, my_gmem, my_open, my_close, my_setbaud, my_init};

struct ioif *DsuUserBackend = &my_io;
