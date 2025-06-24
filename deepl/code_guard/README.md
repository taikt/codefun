# tools
https://www.mermaidchart.com/app/dashboard
https://www.svg2png.com/

# install clang-tidy, cppcheck, clang, npm, typescript
sudo apt-get update
sudo su
apt-get install -y ninja-build build-essential cmake clang git python3 libstdc++-12-dev zlib1g-dev cppcheck clang && sudo apt-get install npm && sudo npm install -g vsce && sudo npm install typescript -g


# build clang-tidy 
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release   -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra"   -DLLVM_TARGETS_TO_BUILD="X86"   -DCMAKE_CXX_FLAGS="-O2" ../llvm

ninja -j4

./bin/clang-tidy --list-checks
./bin/clang-tidy ../src/main.cpp -p ../build-test
./bin/clang-tidy --checks=-*,custom-SingleReturnCheck ./src/main.cpp -p ../build-test
./bin/clang-tidy --checks=-*,custom-SwitchBreakCheck ./../clang-tools-extra/clang-tidy/custom/test.cpp

sudo cp /home/worker/src/llvm-project/build/bin/clang-tidy /usr/local/bin/
clang-tidy --checks=-*,custom-* DAQApplication.cpp

# build vscode extension
npm run compile
vsce package

vsce publish
# https://marketplace.visualstudio.com/manage/publishers/taikt123
# token:
# https://dev.azure.com/tai2tran/_usersSettings/tokens
FIhyEDH4hyvE8B26bKwYc7brGiZwUQjyY4qwLR2TM9p8WpQkY8QtJQQJ99BFACAAAAArP9ilAAASAZDO1UGl
account: tai2.tran@lge.com, LGE PC pw
publisher: taikt123

# delete publisher: 
vsce delete-publisher publisher_id

# change cmake, python version on vbee containter (optional)
sudo unlink /usr/bin/cmake
sudo ln -s ~/cmake-3.22.6/bin/cmake /usr/bin/cmake
sudo apt install -y ninja-build

cd ~/Python-3.9.18
sudo make install
sudo unlink /usr/bin/python
sudo ln -s /usr/local/bin/python3 /usr/bin/python
cd --

# Install Ollama
sudo curl -fsSL https://ollama.com/install.sh | sh

# Download open-source model
ollama pull qwen3:8b
ollama serve &
# Run a model
ollama run qwen3:8b 

# install virtual env
sudo apt update
sudo apt install python3
python3 -m venv ~/src/dl/ollama_env

source /home/worker/src/dl/ollama_env/bin/activate
pip install ollama



## TODO
## copilot history 
Việc kiểm tra open/close file (ví dụ: fopen/fclose, open/close) nên chọn phương pháp như sau:

---

### 1. **Nếu chỉ cần kiểm tra trong phạm vi một hàm (intra-procedural):**
- **clang-tidy custom check** là đủ.
  - Dễ viết, dễ tích hợp.
  - Phù hợp nếu bạn chỉ cần phát hiện trường hợp mở file mà không đóng file trong cùng một hàm.
  - Không phát hiện được nếu file được mở ở một hàm và đóng ở hàm khác.

---

### 2. **Nếu muốn kiểm tra liên hàm (inter-procedural):**
- **Clang Static Analyzer custom check** là lựa chọn tốt nhất.
  - Có thể phát hiện file được mở ở một hàm, truyền qua nhiều hàm, và đóng ở hàm khác.
  - Có thể mở rộng dựa trên cách MallocChecker hoạt động (nhưng cho resource là file).
  - Phức tạp hơn khi viết, nhưng mạnh hơn rất nhiều.

---

### 3. **Tóm lại:**
- **Kiểm tra open/close file trong một hàm:**  
  → **clang-tidy custom check** (nhanh, dễ viết).
- **Kiểm tra open/close file liên hàm, luồng ownership phức tạp:**  
  → **Clang Static Analyzer custom check** (mạnh, chuẩn, phát hiện leak thực sự).

---

Nếu bạn chỉ cần kiểm tra MISRA rule về open/close file trong từng hàm, hãy dùng clang-tidy.  
Nếu bạn muốn phát hiện resource leak thực sự (file không được đóng trên mọi đường đi), hãy dùng Static Analyzer.

Bạn cần ví dụ cho loại nào?

# check memory
MallocChecker.cpp (clang/lib/StaticAnalyzer/Checkers)

# lgedv rule
Dưới đây là phân loại các rule LGEDV nên dùng clang-tidy (AST/logic/style) hay nên dùng Clang Static Analyzer (clang SA, phân tích luồng dữ liệu/resource):

---

### **Nên dùng clang-tidy custom check:**
- **LGEDV_CRCL_0001**: Magic number → kiểm tra literal, enum, define.
- **LGEDV_CRCL_0002**: Duplicate code → kiểm tra code giống nhau (có thể dùng thêm tool khác, nhưng AST check được).
- **LGEDV_CRCL_0003**: Nhiều return trong hàm → đếm ReturnStmt.
- **LGEDV_CRCL_0004**: Giá trị trả về mặc định → kiểm tra khởi tạo biến trả về.
- **LGEDV_CRCL_0005**: Tên biến có ý nghĩa → kiểm tra naming convention.
- **LGEDV_CRCL_0007**: Không dùng thread/looper → kiểm tra gọi thread API.
- **LGEDV_CRCL_0009**: Cyclomatic Complexity → đếm số nhánh, if, for, switch.
- **LGEDV_CRCL_0010**: Giá trị trả về hợp lệ → kiểm tra return type/value.
- **LGEDV_CRCL_0011**: Log đầu hàm → kiểm tra có lệnh log đầu hàm.
- **LGEDV_CRCL_0012**: Sync file sau flush/close → kiểm tra gọi sync sau flush/close (có thể cần analyzer nếu muốn chắc chắn).
- **LGEDV_CRCL_0013**: Switch case phải có break → kiểm tra AST switch/case.
- **LGEDV_CRCL_0014**: Null check pointer, out-of-bound → kiểm tra null check trước deref, index check (có thể cần analyzer nếu muốn chắc chắn).
- **LGEDV_CRCL_0015**: Case nhiều dòng phải có {} → kiểm tra block trong case.
- **LGEDV_CRCL_0016**: Không dùng hàm C không giới hạn → kiểm tra gọi sprintf, strcpy, strcat...
- **LGEDV_CRCL_0018**: Dùng namespace → kiểm tra namespace.
- **LGEDV_CRCL_0019**: Hàm phải trả về giá trị có ý nghĩa → kiểm tra return type/value.

---

### **Nên dùng Clang Static Analyzer (clang SA) custom check:**
- **LGEDV_CRCL_0020**: Resource leak (memory, file, mutex...)  
  → Cần phân tích luồng dữ liệu, ownership, inter-procedural (liên hàm), giống MallocChecker.

---

### **Tóm tắt:**
- **Các rule về style, logic, cấu trúc, naming, return, switch, log, complexity, namespace, API call**  
  → **clang-tidy** (dễ viết, chạy nhanh, phù hợp).
- **Các rule về resource leak, memory leak, ownership, luồng dữ liệu phức tạp**  
  → **Clang Static Analyzer** (phân tích liên hàm, ownership, resource).

---

# Misra rule
Dưới đây là phân loại các rule MISRA C++:2008 (ưu tiên các rule bạn gửi) nên dùng clang-tidy (AST/logic/style) hay nên dùng Clang Static Analyzer (clang SA, phân tích luồng dữ liệu/resource):

---

### **Nên dùng clang-tidy custom check:**
- **6-4-2**: Tất cả if ... else if phải kết thúc bằng else  
  → Kiểm tra cấu trúc if-else trong AST.
- **6-5-1**: For chỉ có một biến đếm, không phải kiểu float  
  → Kiểm tra for loop, biến đếm, kiểu biến.
- **6-5-5**: Biến điều kiện ngoài biến đếm không được thay đổi trong for  
  → Kiểm tra AST for loop, các biến thay đổi trong condition/expression.
- **6-6-2**: Goto chỉ nhảy tới label phía sau  
  → Kiểm tra vị trí label và goto trong cùng hàm.
- **6-6-4**: Mỗi vòng lặp chỉ có tối đa một break hoặc goto để kết thúc  
  → Đếm break/goto trong loop.
- **7-5-1**: Không trả về reference/pointer tới biến tự động  
  → Kiểm tra return type và nguồn gốc biến trả về.
- **7-5-2**: Không gán địa chỉ biến tự động cho biến có vòng đời dài hơn  
  → Kiểm tra assignment, scope của biến.
- **8-4-3**: Mọi đường thoát của hàm non-void phải có return với biểu thức  
  → Kiểm tra return statement trong hàm non-void.
- **8-5-1**: Biến phải được gán giá trị trước khi dùng  
  → Kiểm tra biến được khởi tạo trước khi sử dụng (có thể dùng cả analyzer nếu muốn chắc chắn).
- **10-3-3**: Hàm ảo chỉ được override bởi pure virtual nếu chính nó là pure virtual  
  → Kiểm tra override trong class hierarchy.
- **11-0-1**: Member data trong non-POD phải là private  
  → Kiểm tra access specifier của member.
- **15-0-2**: Exception object không được là pointer  
  → Kiểm tra throw/catch type.
- **16-0-6**: Macro function, mỗi tham số phải có ngoặc  
  → Kiểm tra macro definition.
- **18-0-5**: Không dùng hàm không giới hạn của <cstring>  
  → Kiểm tra gọi hàm như strcpy, strcat, sprintf...

---

### **Nên dùng Clang Static Analyzer (clang SA) custom check:**
- **8-5-1**: Nếu muốn kiểm tra chắc chắn biến đã được gán giá trị trên mọi đường đi phức tạp (dataflow, inter-procedural).
- **7-5-2**: Nếu muốn kiểm tra assignment liên hàm, ownership transfer phức tạp.
- **Các rule về resource leak, memory leak, use-after-free, double free, ownership transfer** (không có trong danh sách bạn gửi, nhưng nếu có thì nên dùng analyzer).

---

### **Tóm tắt:**
- **Đa số rule MISRA C++ về cấu trúc, style, logic, access, macro, override, return, exception, function call**  
  → **clang-tidy** (dễ viết, chạy nhanh, phù hợp).
- **Các rule về luồng dữ liệu phức tạp, resource leak, ownership, khởi tạo biến trên mọi đường đi**  
  → **Clang Static Analyzer** (phân tích liên hàm, dataflow).

---




