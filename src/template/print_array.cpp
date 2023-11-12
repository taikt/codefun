#include <iostream>

using namespace std;

template <int Rows, int Cols>
void printArray2D(int const (&list)[Rows][Cols]) {
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Cols; j++) {
            cout << list[i][j] << " ";
        }
        cout << endl;
    }
}

int main()
{
    int list1[2][2] = { {1,2},{3,4} };

    int row1 = sizeof(list1) / sizeof(list1[0]);
    
    int col1 = sizeof(list1[0]) / sizeof(list1[0][0]);

    printArray2D(list1);

    return 0;
}
 