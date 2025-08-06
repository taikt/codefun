from .views import home
from django.urls import path

urlpatterns = [
    path('', home, name='home'),
    
    

]  # path('form/<int:id>/', form_detail, name='form_detail')
