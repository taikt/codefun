/**
 * Interface Segregation(separation) principle
 * 
 * Asks you to create a different interface for different responsibilities, 
 * in other words, don't group unrelated behavior in one interface, 
 * You break ISP if You have already an interface with many responsibilities, 
 * and the implementor doesn't need all this stuff
 * 
 * https://stackoverflow.com/questions/54480725/the-difference-between-liskov-substitution-principle-and-interface-segregation-p
 */

class Animal
{
public:
    void fly() = 0;
    void eat() = 0;
}

class tiger : public Animal
{
public:
    void eat()
    {
        //OK, I eat sure
    }
    void fly()
    {
        // oh, I am not fly, what the hell?, I don't need this API
    }
}
