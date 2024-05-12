int __attribute__((weak)) a = 4;

void __attribute__((weak)) foo()
{
    a = 5;
}