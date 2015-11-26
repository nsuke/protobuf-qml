import http.server
import my_message_pb2


class MyHandler(http.server.BaseHTTPRequestHandler):
  def do_POST(self):
    self.protocol_version = 'HTTP/1.1'
    length = int(self.headers.get('Content-Length'))
    x = self.rfile.read(length)
    msg = my_message_pb2.MyMessage()
    msg.ParseFromString(x)
    print('Client says: ' + msg.my_text)
    msg.my_text = 'RECEIVED TEXT IS "%s"' % msg.my_text
    data = msg.SerializeToString()
    rsp_len = len(data)
    self.send_response(200)
    self.send_header('Content-Length', rsp_len)
    self.end_headers()
    self.wfile.write(data)


if __name__ == '__main__':
  httpd = http.server.HTTPServer(('127.0.0.1', 34255), MyHandler)
  httpd.serve_forever()
