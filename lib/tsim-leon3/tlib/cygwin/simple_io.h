
extern void tsim_set_ioread(int (*func)(int, int *, int *));
extern void tsim_set_iowrite(int (*func)(int, int *, int *, int));
extern void tsim_set_sigio(void (*func)());

