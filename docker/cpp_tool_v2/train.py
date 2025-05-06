from transformers import AutoModelForSequenceClassification, Trainer, TrainingArguments

# Tải mô hình
model = AutoModelForSequenceClassification.from_pretrained("microsoft/codebert-base", num_labels=len(label_map))

# Định nghĩa tham số huấn luyện
training_args = TrainingArguments(
    output_dir='/content/drive/MyDrive/misra_finetune',
    num_train_epochs=5,  # Tăng epoch
    per_device_train_batch_size=8,
    per_device_eval_batch_size=8,
    warmup_steps=100,
    weight_decay=0.01,
    learning_rate=2e-5,  # Thêm learning rate
    logging_dir='/content/drive/MyDrive/logs',
    logging_steps=10,
    evaluation_strategy="epoch",
     vbnm
      
    save_strategy="epoch",
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

# Huấn luyện
trainer.train()

# Lưu mô hình
model.save_pretrained('/content/drive/MyDrive/misra_finetune_model')
tokenizer.save_pretrained('/content/drive/MyDrive/misra_finetune_model')

# Đánh giá
eval_results = trainer.evaluate()
print(eval_results)