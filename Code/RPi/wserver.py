from flask import Flask
from flask import render_template
from flask import jsonify
from flask import Response

import db
import hike

app = Flask(__name__)
hdb = db.HubDatabase()


## Remove this when using the database
test_data = [{'session_id': 1, 'km': '33', 'steps': '35', 'burnt_kcal': '55'},
            {'session_id': 2, 'km': '433', 'steps': '435', 'burnt_kcal': '555'},
]

@app.route('/')
def get_home():
    sessions = hdb.get_sessions() 
    return render_template('home.html', sessions=test_data)

@app.route('/sessions')
def get_sessions():
    sessions = hdb.get_sessions() 
    sessions = list(map(lambda s: hike.to_list(s), sessions))
    print(sessions)
    return jsonify(sessions)

@app.route('/sessions/<id>')
def get_session_by_id(id):
    session = hdb.get_session(id)
    return jsonify(hike.to_list(session))

@app.route('/sessions/<id>/delete')
def delete_session(id):
    hdb.delete(id)
    print(f'DELETED SESSION WITH ID: {id}')
    return Response(status=202)

if __name__ == "__main__":
    app.run('0.0.0.0', debug=True)