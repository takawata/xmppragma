#pragma xmp nodes p[4]
#pragma xmp template t[64]
#pragma xmp distribute t[block] onto p

int a(double x[64])
{
#pragma xmp align x[i] with t[i]
#pragma xmp loop (i) on t[i]
  for(int i = 0; i < 64; i++){
    x[i] = 0;
  }
  return 0;
}

int main()
{
  double foo[64];
#pragma xmp align foo[i] with t[i]
  a(foo);
  
}
