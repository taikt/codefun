class Solution {
public:
    // int jobDone = 0;
    int currentNumberOfSession;
    vector<int> memorize;
    int allJobDoneMask;
	
	//Thay vì sử dụng một mảng bool để đánh dấu, ta sử dụng 1 dãy bit
	//Việc sử dụng dãy bit này sẽ giúp lưu số session đã trải qua để đạt được trạng thái công việc tương ứng thông qua index của mảng memorize

    bool checkBitAtIndex(int number, int bitIndex) {
        return ((number >> bitIndex) & 1) == 1;
    }

    void setBitAtIndex(int &number, int bitIndex) {
        number = (number | (1 << bitIndex));
    }

    void resetBitAtIndex(int &number, int bitIndex) {
        number = (number & (~(1 << bitIndex)));
    }

    void doInNewSession(vector<int>& tasks, int done, int sessionTime) {
		//Nếu tất cả công việc đã hoàn thành, không bắt đầu session này
        if(done == allJobDoneMask) {
            return;
        }
        currentNumberOfSession++;

        for(int jobWillDoneInThisSession = allJobDoneMask; jobWillDoneInThisSession > done; jobWillDoneInThisSession--) {
			//Nếu trạng thái jobWillDoneInThisSession không dựa trên trạng thái hiện tại ( done) thì không thể đi từ trang trạng hiện tại sang trạng thái của jobWillDoneInThisSession được
			//Nếu với trạng thái jobWillDoneInThisSession mà số session để đạt được nó vốn đã thấp hơn hoạc bằng với con số đang có thì việc chạy tiếp với trạng thái này không cần thiết nữa
            if((jobWillDoneInThisSession & done) != done || memorize[jobWillDoneInThisSession] <= currentNumberOfSession) continue;
            int jobTime = 0;
			//Tính tổng thời gian làm cho session này
            for(int i = 0; i < tasks.size(); i++) {
                if(checkBitAtIndex(jobWillDoneInThisSession, i) == true && checkBitAtIndex(done, i) == false) {
                    jobTime += tasks[i];
                }
            }
            if(jobTime <= sessionTime) {
				//Nhờ bước kiểm tra ở trên mà chắc chắn rằng trạng thái jobWillDoneInThisSession sẽ có giá trị tốt hơn giá trị đã lưu
				//Ta sẽ lưu lại giá trị cho trạng thái này đồng thời tiếp tục với session mới
                memorize[jobWillDoneInThisSession] = currentNumberOfSession;
                doInNewSession(tasks, jobWillDoneInThisSession, sessionTime);
            }
        }
        currentNumberOfSession--;
    }

    int minSessions(vector<int>& tasks, int sessionTime) {
        currentNumberOfSession = 0;
        int done = 0;
        allJobDoneMask = (1 << tasks.size()) - 1;
		//Khởi tạo mảng này với giá trị chác chắn lớn hơn kết quả cuối để bỏ qua bước kiểm tra đã từng check trạng thái công việc này đã từng duyệt qua chưa
        memorize = vector<int>(1 << tasks.size(), 100);
		doInNewSession(tasks, done, sessionTime);
        return memorize[allJobDoneMask];
    }
};