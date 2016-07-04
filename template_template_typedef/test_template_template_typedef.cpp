


template <unsigned int i, typename A, template <typename> class TT>
struct Test
{
    typedef typename Test<i-1, A, TT>::Down Down;
};

template <typename T, template <typename> class TT>
struct Test<0,T,TT>
{
    typedef int Down;
};

template <typename T>
struct X
{
    typedef char P;
};



int main()
{
    Test<3,int,X> test;

    
}
