class Solution {
public:
    //Tìm tất cả các thử tự lựa chọn có thể 
    //Với mỗi thứ tự tính tổng điểm
    //Sau tất cả các lần thử thì in ra kết quả lớn nhât
    int result;
    int currVal;
    unordered_map<int, int> table;
    //vector<int> table(1 << 14);

    //Round 1 lấy 0th và 1st, Round 2 lấy 2nd, 3rd
    //Round 1 lấy 2nd, 3rd, Round 2 lấy 0th và 1st
    //Nhận thấy có nhiều bược bị lặp không cần thiết
    //=> Lưu các trạng thái sau mỗi round, chỉ thự hiện nếu trạng thái mơi tốt hơn => Dynamic programming

    bool isBitSet(int num, int bitIndex) {
        return ((num >> bitIndex) & 1) == 1;
    }

    void setBit(int &num, int bitIndex) {
        num = (num | (1 << bitIndex));
    }

    void resetBit(int &num, int bitIndex) {
        num = (num & (~(1 << bitIndex)));
    }

    void run(int round, vector<int>& nums, int used) {
        //Kết thúc đệ quy
        if(round * 2 > nums.size()) {
            result = max(currVal, result);
            return;
        }
        //Tạo ra bộ 2 số để tính
        for(int i = 0; i < nums.size(); i++) {
            //Chọn số đầu tiên
            if(isBitSet(used, i)) continue;
            // used[i] = true;
            setBit(used, i);
            for(int j = i + 1; j < nums.size(); j++) {
                //Chọn số thứ 2
                if(isBitSet(used, j)) continue;
                setBit(used, j);
                int val = round * gcd(nums[i], nums[j]);
                currVal += val;
                //Hoàn thành 1 round
                if(table.find(used) == table.end() || table[used] < currVal) {
                    table[used] = currVal;
                    run(round + 1, nums, used);
                }
                currVal -= val;
                // used[j] = false;
                resetBit(used, j);
            }
            // used[i] = false;
            resetBit(used, i);
        }
    }

    int maxScore(vector<int>& nums) {
        result = 0;
        currVal = 0;
        //vector<bool> used(nums.size(), false);
        int used = 0;
        //Liệt kê tất cả các thứ tự
        run(1, nums, used);
        return result;
    }
};