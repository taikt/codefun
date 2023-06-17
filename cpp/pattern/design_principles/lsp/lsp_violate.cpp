/**
 * LSP: Liskov substituation priciple
 * given T is base class
 * suppose we want class T inherit from S:
 * then: LSP said T must can use all methods from S
 * if not, it violate LSP and we should not let T inherit from S
 * 
 * https://stackoverflow.com/questions/56860/what-is-an-example-of-the-liskov-substitution-principle
 */

class Bird
{
public:
    void fly() {}
};
/**
 * Duck inherit Bird don't violate LSP
 * because Duck can use fly method (truly Duck can fly)
 * @param  {c []} undefined : 
 */
class Duck : public Bird
{
public:
    void fly()
    {
        cout << "duck can fly, ok";
    }
}

/**
 * Ostrich inherit Bird: violate LSP
 * because Ostrich(da dieu) cannot fly, so it cannot use fly() method
 * @param  {c []} undefined : 
 */
class Ostrich : public Bird
{
public:
    void fly()
    {
        cout << "hmm,..ostrich can't fly";
    }
}
