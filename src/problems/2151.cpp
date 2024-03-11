class Solution {
public:

    const int GOOD = 1;
    const int BAD = 0;
    const int UNK = 2;

    int result;

    // Tìm số người tốt tối đa
    // ??? 1 phân phối/kiểu hình sao cho số người tốt trong đó lớn nhất
    // Tối đa 2 ^ 15 kiểu hình khác nhau của tập người
    // => Thử gen ra toàn bộ các kiểu hình của tập người
    // Với mỗi kiểu hình kiểm tra xem là nó có phù hợp với tập câu trả lời hay không
    // Nếu phù hợp thì đếm số người tốt và xem xem kết quả này đã lớn nhất hay chưa

    bool check(vector<vector<int>>& statements, vector<int> &personality) {
        //Lời nói của người xấu không quan trọng => chỉ kiểm tra người tốt nói có chính xác không
        for(int i = 0; i < personality.size(); i++) {
            if(personality[i] == BAD) continue;
            for(int j = 0; j < personality.size(); j++) {
                //Kiểm tra từng lời nói này đúng hay không
                if(statements[i][j] == UNK) continue;
                if(statements[i][j] == GOOD && personality[j] == BAD) return false;
                if(statements[i][j] == BAD && personality[j] == GOOD) return false;
            }
        }
        return true;
    }

    void create(int index, vector<vector<int>>& statements, vector<int> &personality) {
        if(index == statements.size()) {
            //Check xem kiểu hình này phù hợp vs statements không
            if(check(statements, personality)) {
                //đếm số người tốt và so với kết quả cuối cùng
                int thisResult = 0;
                for(int &i: personality) {
                    if(i == GOOD) thisResult++;
                }
                result = max(result, thisResult);
            }
            return;
        }
        //Người thử "index" có 2 trường hợp
        //1 người tốt
            // Chọn trường hợp cho người "index + 1"
        personality[index] = GOOD;
        create(index + 1, statements, personality);
        //2 người xấu
            // Chọn trường hợp cho người "index + 1"
        personality[index] = BAD;
        create(index + 1, statements, personality);
    }

    int maximumGood(vector<vector<int>>& statements) {
        result = 0;
        vector<int> personality(statements.size());
        //Gen ra tất cả các kiểu hình
        create(0, statements, personality);
        return result;
    }

    //for (int i = 0; i < (1 << statements.size()); i++) { // trich xuất bit để ra kiểu hình}
};