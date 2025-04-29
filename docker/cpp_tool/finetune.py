from transformers import AutoTokenizer
from datasets import load_dataset # Import load_dataset

tokenizer = AutoTokenizer.from_pretrained("microsoft/codebert-base")

def tokenize_function(examples):
    return tokenizer(examples['code'], padding="max_length", truncation=True, max_length=512)

# Load the dataset before calling .map()
dataset = load_dataset('json', data_files='/content/drive/MyDrive/misra_dataset.json')
tokenized_dataset = dataset.map(tokenize_function, batched=True)

# Mã hóa nhãn
label_map = {
    "MISRA C++ 5-0-1": 0,
    "MISRA C++ 3-1-1": 1
}
def encode_labels(example):
    if example["labels"] in label_map:
        example["labels"] = label_map[example["labels"]]
        return example
    else:
        return None  # hoặc xử lý phù hợp

tokenized_dataset = tokenized_dataset.map(encode_labels)


# Training process

from transformers import AutoModelForSequenceClassification, Trainer, TrainingArguments, IntervalStrategy

# Tải mô hình đã huấn luyện trước
model = AutoModelForSequenceClassification.from_pretrained("microsoft/codebert-base", num_labels=2)

# Định nghĩa các tham số huấn luyện
training_args = TrainingArguments(
    output_dir='/content/drive/MyDrive/misra_finetune',
    num_train_epochs=3,
    per_device_train_batch_size=8,
    per_device_eval_batch_size=8,
    warmup_steps=100,
    weight_decay=0.01,
    logging_dir='/content/drive/MyDrive/logs',
    logging_steps=10,
    eval_strategy=IntervalStrategy.EPOCH,  # Đánh giá sau mỗi epoch
    save_strategy=IntervalStrategy.EPOCH,  # Lưu mô hình sau mỗi epoch
    load_best_model_at_end=True,
    remove_unused_columns=False,
    fp16=True
)
# Khởi tạo Trainer
trainer = Trainer(
    model=model,
    args=training_args,
    train_dataset=train_dataset,
    eval_dataset=eval_dataset
)

trainer.train()
