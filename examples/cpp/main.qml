import QtQuick 2.5
import 'cpp_example.pb.js' as Ex

Item {
  // TODO: Add some GUI
  Component.onCompleted: {
    var foo = new Ex.Foo({
      text: 'Lorem ipsum dolor sit amet, ...',
      repeat: 42,
    });

    // Pass QML protobuf message to C++ layer.
    var raw = someCppObject.doSomeNativeStuff(foo);

    // Currently, we need to explicitly call the constructor
    var bar = new Ex.Bar(raw);

    console.log('Printing return values from C++');
    for (var i = 0; i < bar.textsSize; ++i) {
      console.log(bar.texts(i));
    }
    console.log('done printing ' + i + ' lines');
  }
}
