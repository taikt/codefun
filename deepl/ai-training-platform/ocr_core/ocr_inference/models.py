from django.db import models

# Create your models here.
class InferModel(models.Model):
    code = models.CharField(max_length=255, unique=True)
    inference_path = models.CharField(max_length=255)
    # config_path = models.CharField(max_length=255)

