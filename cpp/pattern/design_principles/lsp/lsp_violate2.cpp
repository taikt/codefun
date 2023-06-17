/**
 * LSP (Liskov Substitution): this principle asks you to make sure that all child classes
 *  have the same behavior as the parent class. for example: if you have a Device class 
 * and it has function callBaba() which get your father phone number then calls him, 
 * So you must make sure that the callBaba() method in all subclasses of the Device 
 * does the same job. if any of the subclasses of  Device have another behavior
 *  inside callBaba() this is mean you broke the LSP
 * 
 * https://stackoverflow.com/questions/54480725/the-difference-between-liskov-substitution-principle-and-interface-segregation-p
 */

/**
 * Violate LSP example
 * @param  {c []} undefined : 
 */
class Device
{
    func callBaba()
    {
        print("I will find your father and I will call him")
    }
}

class Calculator : Device
{
    override func callBaba()
    {
        print("Sorry, I don't have this functionality ")
    }
}

/**
 * Solution to avoid violate LSP
 * @param  {i []} undefined : 
 */
interface CanCall
{
    func callBaba()
}
class Device
{
    // all basic shared functions here.
}

class Calculator : Device
{
    // all functions that calculator can do + Device
} class SmartPhone : Device implements CanCall
{
    // all smartphone stuff
    func callBaba()
    {
        print("I will find your father and I will call him")
    }
}