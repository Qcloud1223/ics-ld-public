int array[2] = {1, 2};

int sum(int *a)
{
    return a[0] + a[1];
}

int main()
{
    return sum(array);
}