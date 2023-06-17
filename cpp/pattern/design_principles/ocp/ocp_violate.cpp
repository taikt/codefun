/**
 * OCP: open-closed principle
 * open cho extension code, nhung closed neu modification code
 * 
 * Phân tích vi pham OCP:
 * neu them bo loc moi vd byNameAndColor: can them code
 * thêm yêu cầu bộ lọc mơi cho thuộc tính mới (ie nguồn sản phẩm (origin_)): phai modify code
 * 
 * cls: to clear terminal (visual studio code)
 * g++ ocp_violate.cpp --std=c++11
 * a.exe
 * 
 */

#include <iostream>
#include <string>
#include <vector>

using namespace std;

/*
// C++ style
enum class COLOR
{
  RED,
  GREEN,
  BLUE
};
enum class SIZE
{
  SMALL,
  MEDIUM,
  LARGE
};
*/

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
//typedef vector<Product *> Items;

class ProductFilter
{
public:
    /**
     * Loc san phan by colorcls
     * 
     * @param  {Items} items : tat ca san pham dau vao
     * @param  {Color} color : mau sac can loc
     * @return {Items}       : cac san pham duoc loc
     */
    static Items byColor(Items items, Color color)
    {
        Items result;
        for (auto &i : items)
        {
            if (i->color_ == color)
            {
                result.push_back(i);
            }
        }
        return result;
    }

    static Items bySize(Items items, Size size)
    {
        Items result;
        for (auto &i : items)
        {
            if (i->size_ == size)
            {
                result.push_back(i);
            }
        }
        return result;
    }

    static Items byColorAndSize(Items items, Color color, Size size)
    {
        Items result;
        for (auto &i : items)
        {
            if (i->color_ == color && i->size_ == size)
            {
                result.push_back(i);
            }
        }
        return result;
    }
};

int main()
{
    const Items inputProducts{
        new Product{"X1", GREEN, SMALL},
        new Product{"X2", RED, LARGE},
        new Product{"X3", RED, MEDIUM},
    };
    for (auto &p : ProductFilter::byColor(inputProducts, RED))
    {
        cout << "product:" << p->name_ << " is red\n";
    }

    return 0;
}
