#include <iostream>
#include <string>
#include <vector>
using namespace std;

enum Color
{
    RED,
    GREEN,
    BLUE
};

enum Size
{
    SMALL,
    MEDIUM,
    LARGE
};

class Product
{
public:
    string name_;
    Color color_;
    Size size_;
};

/*
class Mapper
{
public:
    virtual bool satisfy(Product *p) = 0;
};
*/

// change Product with T
template <typename T>
class Mapper
{
public:
    virtual bool satisfy(T *p) = 0;
};

class ColorMapper : public Mapper<Product>
{
public:
    ColorMapper(Color color) : color_(color) {}
    bool satisfy(Product *p)
    {
        return (p->color_ == color_);
    }

private:
    Color color_;
};

class SizeMapper : public Mapper<Product>
{
public:
    SizeMapper(Size size) : size_(size) {}
    bool satisfy(Product *p)
    {
        return (p->size_ == size_);
    }

private:
    Size size_;
};

template <typename T>
class Filter
{
public:
    virtual vector<T *> filter(vector<T *> items, Mapper<T> &mapper) = 0;
};

class ProductFilter : public Filter<Product>
{
public:
    vector<Product *> filter(vector<Product *> items, Mapper<Product> &mapper)
    {
        vector<Product *> result;
        for (auto &p : items)
        {
            if (mapper.satisfy(p))
            {
                result.push_back(p);
            }
        }
        return result;
    }
};

template <typename T>
class AndMapper : public Mapper<T>
{
public:
    AndMapper(Mapper<T> &first, Mapper<T> &sec) : first_(first), sec_(sec) {}

    bool satisfy(T *p)
    {
        return (first_.satisfy(p) && sec_.satisfy(p));
    }

private:
    Mapper<T> &first_;
    Mapper<T> &sec_;
};

int main()
{

    const vector<Product *> inputProducts{
        new Product{"X1", GREEN, SMALL},
        new Product{"X2", RED, LARGE},
        new Product{"X3", RED, MEDIUM},
    };

    ProductFilter bf;
    ColorMapper red(RED);
    SizeMapper medium(MEDIUM);
    AndMapper<Product> andMap(red, medium);

    for (auto &p : bf.filter(inputProducts, red))
    {
        cout << "product:" << p->name_ << " is red \n";
    }

    for (auto &p : bf.filter(inputProducts, andMap))
    {
        cout << "product:" << p->name_ << " is red and medium \n";
    }

    return 0;
}