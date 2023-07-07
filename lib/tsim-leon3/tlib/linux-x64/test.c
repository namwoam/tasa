main()
{
  int i, sum;
  volatile int *leon = (int *) 0x80000000;
  volatile int *ioarea = (int *) 0x20000000;
	   
   leon[0] |= 0x800;
    for (i=0; i<0x10000; i++) {
      if (!(i & 0x01fff)) {
        printf("sum = %d\n", i);
	*ioarea = i;
      }
    }
		 
}
