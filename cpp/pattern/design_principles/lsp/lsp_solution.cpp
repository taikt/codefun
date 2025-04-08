class Bird
{
public:
    void hi() {}
};
/**
 * declare a separate class for flying birds
 * @param  {c []} undefined : 
 */
class flyingBirds : public Bird
{
public:
    void hi()
    {
        cout << "I said hi";
    }
    void fly()
    {
        cout << "flyingBirds can fly, ok";
    }
}

class Duck : public flyingBirds
{
public:
    void fly()
    {
        cout << "Duck can fly, ok";
    }
}

/**
 * Ostrich inherit Bird: not violate LSP
 * because Bird now don't have fly() method
 * @param  {c []} undefined : 
 */
class Ostrich : public Bird
{
public:
    void hi()
    {
        cout << "I ostrick said hi";
    }
}