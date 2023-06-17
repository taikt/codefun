/**
 * to not violate ISP: let separate interfaces, don't group unrelated ones
 * @param  {c []} undefined : 
 */
class Animal
{
public:
    void hi() = 0;
}

class flyableAnimal : public Animal
{
    void hi() {}
    void fly() = 0;
}

class feedabeAnimal : public Animal
{
    void hi() {}
    void eat() = 0;
}

class tiger : public feedabeAnimal
{
public:
    void eat()
    {
        //OK, I eat sure
    }
}