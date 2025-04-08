/**
 * principle to apply OCP:
 * use abstract class, inheritance based on abstract class
 * 
 * @param  {# []} undefined : 
 */
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

using Items = vector<Product *>;

class Mapper
{
public:
    virtual bool satisfy(Product *p) = 0;
};

class ColorMapper : public Mapper
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

/**
 * if there's new mapper: just add like this mapper: apply rule "open for extension" 
 * @param  {c []} undefined : 
 */
class SizeMapper : public Mapper
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

class Filter
{
public:
    virtual Items filter(Items items, Mapper &mapper) = 0;
    //virtual Items filter(vector<Product *> items, Mapper &mapper) = 0;
};

/**
 * this class don't need to modify if there's a new mapper: apply rule "closed for modification"
 * @param  {c []} undefined : 
 */
class ProductFilter : public Filter
{
public:
    //vector<Product *> filter(vector<Product *> items, Mapper &mapper)
    Items filter(Items items, Mapper &mapper)
    {
        Items result;
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

class AndMapper : public Mapper
{
public:
    AndMapper(Mapper &first, Mapper &sec) : first_(first), sec_(sec) {}

    bool satisfy(Product *p)
    {
        return (first_.satisfy(p) && sec_.satisfy(p));
    }

private:
    Mapper &first_;
    // Mapper first_; ==> wrong compile because Mapper is abstract class, cann't be instantiated,
    // need to use reference or pointer
    Mapper &sec_;
};

int main()
{

    const Items inputProducts{
        new Product{"X1", GREEN, SMALL},
        new Product{"X2", RED, LARGE},
        new Product{"X3", RED, MEDIUM},
    };

    ProductFilter bf;
    ColorMapper red(RED);
    SizeMapper medium(MEDIUM);
    AndMapper andMap(red, medium);

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