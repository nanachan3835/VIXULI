import requests

while True:
    try:
        print(requests.get('http://192.168.1.240/test?param1=1&param2=2', timeout=20).text)
        input()
    except requests.exceptions.ReadTimeout as e:
        continue