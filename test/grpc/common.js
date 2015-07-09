.pragma library
'use strict';

var test_count = 4;

function test_unary(test, data) {
  for (var i = 0; i < test_count; ++i) {
    var val = {};
    // when invoked service
    var ok = data.client.sayHello({name: 'Foo'}, function(err, rsp) {
      // should not receive errror
      test.verify(!err, err);

      // should receive response processed by server
      test.compare(rsp.greet(), 'Hello Foo');
      val.called = true;
    }, 500);

    // should be able to send
    test.verify(ok);

    // should receive response
    test.tryCompare(val, 'called', true, 2000);
  }
}

function test_client_streaming(test, data) {
  for (var i = 0; i < test_count; ++i) {
    var val = {};
    // when start calling service
    var call = data.client.batchHello(function(err, rsp) {
      // should not receive errror
      test.verify(!err, err);

      // should receive response processed with all messages by server
      test.compare(rsp.greet(), 'Hello Bar1 Bar2 Bar3');
      val.called = true;
    });

    call.timeout = 500;

    // and send 3 messages and not end
    var ok = call.write({name: 'Bar1'});
    test.verify(ok);
    ok = call.write({name: 'Bar2'});
    test.verify(ok);
    ok = call.write({name: 'Bar3'});
    test.verify(ok);

    // should not receive response yet
    test.verify(!val.called);

    // when notified wirtes done
    ok = call.end(1000);

    // should receive response
    test.tryCompare(val, 'called', true);
  }
}

function test_server_streaming(test, data) {
  for (var i = 0; i < test_count; ++i) {
    var end = {};
    var received = [];

    var ok = data.client.subscribeHello({
      requests: [{
        name: 'Baz0',
      } , {
        name: 'Baz1',
      } , {
        name: 'Baz2',
      } , {
        name: 'Baz3',
      }],
    }, function(err, data, finished) {
      test.verify(!err, err);
      if (finished) {
        end.called = true;
      } else {
        received.push(data.greet());
      }
    });
    test.verify(ok);

    test.tryCompare(end, 'called', true);
    test.compare(received.length, 4);
    test.compare(received[0], 'Hello Baz0');
    test.compare(received[1], 'Hello Baz1');
    test.compare(received[2], 'Hello Baz2');
    test.compare(received[3], 'Hello Baz3');
  }
}

function test_bidi_streaming(test, data) {
  for (var i = 0; i < test_count; ++i) {
    var end = {};
    var received = [];

    var call = data.client.bidiHello(function(err, data, finished) {
      test.verify(!err, err);
      if (finished) {
        end.called = true;
      } else {
        received.push(data.greet());
      }
    });
    test.verify(call);

    // Ensure that call succeeds even if client does not immediately write.
    test.wait(100);

    var res = call.write({
      requests: [{
        name: 'Baz0',
      } , {
        name: 'Baz1',
      } , {
        name: 'Baz2',
      } , {
        name: 'Baz3',
      }],
    });
    test.verify(res);

    res = call.write({
      requests: [{
        name: 'Foo0',
      } , {
        name: 'Foo1',
      } , {
        name: 'Foo2',
      }],
    });
    test.verify(res);

    res = call.end();
    test.verify(res);

    test.tryCompare(end, 'called', true);
    test.compare(received.length, 7);
    test.compare(received[0], 'Hello Baz0');
    test.compare(received[1], 'Hello Baz1');
    test.compare(received[2], 'Hello Baz2');
    test.compare(received[3], 'Hello Baz3');
    test.compare(received[4], 'Hello Foo0');
    test.compare(received[5], 'Hello Foo1');
    test.compare(received[6], 'Hello Foo2');
  }
}
