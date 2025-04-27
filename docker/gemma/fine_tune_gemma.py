import json
import torch
from transformers import AutoModelForCausalLM, AutoTokenizer, Trainer, TrainingArguments
from datasets import load_dataset
from peft import LoraConfig, get_peft_model
from pathlib import Path

# Hàm tạo prompt và response
def create_prompt(example):
    return f"""
    Dưới đây là đoạn mã C++:
    ```cpp
    {example['code']}
    ```
    Kiểm tra đoạn mã này dựa trên quy tắc {example['rule_id']}: "{example['description']}"
    Trả về JSON với các trường: violation (true/false), description, suggestion.
    """

def create_response(example):
    return json.dumps({
        "violation": example["violation"],
        "description": example["description"],
        "suggestion": example["suggestion"]
    })

# Tải mô hình và tokenizer
model_name = "google/gemma-2b"  # Mô hình nhỏ phù hợp với M1 Pro
access_token="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"
tokenizer = AutoTokenizer.from_pretrained(model_name, token=access_token)

model = AutoModelForCausalLM.from_pretrained(
    model_name,
    device_map="mps" if torch.backends.mps.is_available() else "cpu",
    torch_dtype=torch.float16,
    token=access_token
)

# Cấu hình LoRA
lora_config = LoraConfig(
    r=8,
    lora_alpha=32,
    target_modules=["q_proj", "v_proj"],
    lora_dropout=0.05,
    bias="none",
    task_type="CAUSAL_LM"
)
model = get_peft_model(model, lora_config)

# Tải dữ liệu
dataset_path = "/app/output/misra_dataset.json"
if not Path(dataset_path).exists():
    raise FileNotFoundError(f"Dataset not found at {dataset_path}. Run generate_misra_data.py first.")
dataset = load_dataset("json", data_files=dataset_path)

# Tiền xử lý dữ liệu
def preprocess_function(examples):
    prompts = [create_prompt(example) for example in examples]
    responses = [create_response(example) for example in examples]
    inputs = tokenizer(prompts, padding="max_length", truncation=True, max_length=512)
    outputs = tokenizer(responses, padding="max_length", truncation=True, max_length=128)
    return {
        "input_ids": inputs["input_ids"],
        "attention_mask": inputs["attention_mask"],
        "labels": outputs["input_ids"]
    }

tokenized_dataset = dataset.map(preprocess_function, batched=True)

# Cấu hình huấn luyện
training_args = TrainingArguments(
    output_dir="/app/output/gemma2b-misra-finetuned",
    per_device_train_batch_size=1,  # Giảm batch size cho M1 Pro
    gradient_accumulation_steps=8,
    num_train_epochs=3,
    learning_rate=2e-4,
    fp16=True,
    logging_steps=10,
    save_steps=100,
    save_total_limit=2,
    report_to="none"  # Tắt gửi báo cáo đến các dịch vụ như WandB
)

# Khởi tạo Trainer
trainer = Trainer(
    model=model,
    args=training_args,
    train_dataset=tokenized_dataset["train"],
)

# Bắt đầu fine-tuning
trainer.train()

# Lưu mô hình
model.save_pretrained("/app/output/gemma2b-misra-finetuned")
tokenizer.save_pretrained("/app/output/gemma2b-misra-finetuned")
print("Fine-tuning completed. Model saved to /app/output/gemma2b-misra-finetuned")