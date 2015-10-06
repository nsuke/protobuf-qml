#include "protobuf/qml/conversions.h"
#include "cpp_example.pb.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <iostream>

class ExampleCppObject : public QObject {
  Q_OBJECT

public:
  Q_INVOKABLE QJSValue doSomeNativeStuff(QJSValue arg1) {
    // Prepare converter that uses QML engine associated with this QJSValue.
    auto convert = protobuf::qml::JSValueConverter::fromQJSValue(arg1);
    if (!convert) {
      qWarning() << "Failed to initialize JS value converter.";
      return QJSValue();
    }

    // Convert to C++ protobuf message.
    Foo foo;
    if (!convert->fromJSValue(foo, arg1)) {
      qWarning() << "Failed to convert to C++ Protobuf message.";
      return QJSValue();
    }

    // Do whatever
    auto bar = doSomethingInternal(foo);

    // Convert back to QML protobuf message.
    auto jsValue = convert->toJSValue(bar);
    if (jsValue.isUndefined()) {
      qWarning() << "Failed to convert to QML Protobuf message.";
    }

    return std::move(jsValue);
  }

private:
  Bar doSomethingInternal(const Foo& foo) {
    std::string str;
    // Needless serialization for demonstration.
    foo.SerializeToString(&str);
    std::cout << "Serialized Foo is " << str << std::endl;

    // Silly construction of return value
    Bar bar;
    for (decltype(foo.repeat()) i = 0; i < foo.repeat(); ++i) {
      bar.add_texts(foo.text());
    }

    // Needless serialization for demonstration again.
    bar.SerializeToString(&str);
    std::cout << "Serialized Bar is " << str << std::endl;
    return std::move(bar);
  }
};

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;
  ExampleCppObject obj;
  engine.rootContext()->setContextProperty("someCppObject", &obj);
  engine.load(app.applicationDirPath() + "/../examples/cpp/main.qml");

  return app.exec();
}

#include "main.moc"
