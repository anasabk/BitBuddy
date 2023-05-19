Kendi bilgisayarınızda denemek için:

## Gerekli packageleri yükleyin:
$ sudo pip install python-opencv python-numpy


## pattern.png dosyasındaki resmi bir çıkartın. (Telefon ekranından açabilirsiniz.)
Çıkarttığınız yerde karelerin kaç milimetro olduğunu olçün.


## Programı şu şekilde çalıştırın. height ve width kamera çözünürlüğü olacak.:
## Açılan kameradan farklı bir kamerayla çalıştırmak için source = cv2.VideoCapture(0) satırındaki parametreyi değiştirin.
$ calibrate.py --mm <milimetre> --width <genişlik> --height <yükseklik> 
örnek:
$ calibrate.py --mm 10 --width 640 --height 480
## Çalıştırırken missing module hataları verebilir, o modülleri de aratıp yükleyin.


## Çalıştırdıktan sonra kameraya pattern resmini farklı açılardan gösterin. Program uygun anlarda ekran görüntüleri topladıktan sonra kalibrasyon yapacak.


## Kalibrasyon bitince program camera matrix ve distortion coefficients bastıracak. Bunun yanında calibrationfiles klasörüne sonuçları .txt ve .pkl olarak kaydedecek.




Raspberry'de denemek için video:
https://youtu.be/EBr9OlxyP9c
