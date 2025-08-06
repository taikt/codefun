"""
URL configuration for ocr_core project.

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/5.1/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import include, path
from django.conf import settings
from django.conf.urls.static import static
from training_engine import views
urlpatterns = [
    path("admin/", admin.site.urls),
    path('train/', include('training_engine.urls')), 
    path('infer/', include('ocr_inference.urls')), 
    path('license/', include('license.urls')), 
    # path('/', views.home, name = 'home'),
    path('get-csrf-token/', views.get_csrf_token, name='get_csrf_token'),
]
