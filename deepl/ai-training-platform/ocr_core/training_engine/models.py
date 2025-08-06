from django.db import models
from datetime import datetime
# Create your models here.
class TrainSession(models.Model):
    session_name = models.CharField(max_length=255, unique=True)
    code = models.CharField(max_length=255)
    status = models.CharField(max_length=128,default=False)
    progress = models.IntegerField(default=0)
    model_path = models.CharField(max_length=255)
    inference_path = models.CharField(max_length=255)
    start_time = models.DateTimeField(default=None)
    end_time = models.DateTimeField(default=None)
    accuracy = models.FloatField(default=0)
    pid = models.IntegerField()
    

    