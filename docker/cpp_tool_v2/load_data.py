from transformers import AutoTokenizer
from datasets import load_dataset

# Khởi tạo tokenizer
tokenizer = AutoTokenizer.from_pretrained("microsoft/codebert-base")

def tokenize_function(examples):
    return tokenizer(examples['code'], padding="max_length", truncation=True, max_length=512)

# Load dataset
dataset = load_dataset('json', data_files='/content/drive/MyDrive/misra_dataset.json')

# Kiểm tra nhãn trong dataset
def check_labels(dataset):
    labels = set(dataset['train']['label'])
    print("Các nhãn trong dataset:", labels)
    return labels

check_labels(dataset)

# Định nghĩa label_map
label_map = {
    "No violation": 0,
    "Rule 5-0-1": 1,
    "Rule 5-0-2": 2,
    "Rule 6-4-1": 3,
    "Rule 8-4-1": 4,
    "Rule 3-1-1": 5
}

# Lọc các mẫu có nhãn hợp lệ
def filter_valid_labels(example):
    return example["label"] in label_map

filtered_dataset = dataset.filter(filter_valid_labels)

# Tokenize dataset
tokenized_dataset = filtered_dataset.map(tokenize_function, batched=True)

# Mã hóa nhãn
def encode_labels(example):
    example["labels"] = label_map[example["label"]]
    return example

tokenized_dataset = tokenized_dataset.map(encode_labels)

# Chia dataset
dataset = tokenized_dataset['train'].train_test_split(test_size=0.2)
train_dataset = dataset['train']
eval_dataset = dataset['test']

# In thông tin dataset
print("Số mẫu trong tập huấn luyện:", len(train_dataset))
print("Số mẫu trong tập đánh giá:", len(eval_dataset))