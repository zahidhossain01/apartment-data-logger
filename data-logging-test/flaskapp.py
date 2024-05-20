from flask import Flask, request, make_response
import time

app = Flask(__name__)

@app.route("/logdata", methods=['POST']) # change to POST
def log_data():
    data = request.get_data()
    print(data)
    # resp = make_response(201)
    return "success"


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)
