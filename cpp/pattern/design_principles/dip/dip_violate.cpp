/**
 * DIP: dependency inversion principle
 * 
 * Depend upon abstraction(interfaces), not upon concreate classes
 * 
 * High-level modules should not depend on low-level modules. Both should depend on abstractions.
 * Abstractions should not depend on details. Details should depend on abstractions.
 */

// violate DIP
enum OutputDevice
{
    printer,
    disk
};
/**
 * this code violates DIP
 * if number of outputDevice changes, we must change copy code
 * now copy() code depends on concrete class: printer, disk, writePrinter, writeDisk
 * @param  {OutputDevice} dev : 
 */
void copy(OutputDevice dev)
{
    int c;
    while ((c = readKeyboard() != eof))
    {
        if (dev == printer)
        {
            writePrinter(c);
        }
        else
        {
            writeDisk(c);
        }
    }
}

// Solution

interface Reader
{
    char read();
}

interface Write
{
    char write(char ch);
}
/**
 * Reader and writer are dependencies of copy()
 * copy() don't create Writer object
 * it inverts responsibility and leave it to user
 * User is free to pass to copy() method its defined Reader and Writer object
 * 
 * event user changes reader and writer method in future,
 * this copy method don't need to change
 * 
 * @param  {Reader} r : 
 * @param  {Writer} w : 
 */
void copy(Reader r, Writer w)
{
    int ch;
    while ((ch = r.read() != eof))
    {
        w.write(ch);
    }
}
