#include "protobuf/qml/conversions.h"
#include "test_convert.pb.h"

#include <QCoreApplication>
#include <QQuickItem>
#include <QQuickView>
#include <QTest>
#include <QtQml>

using namespace protobuf::qml;

class SomeQmlType : public QObject {
  Q_OBJECT

public:
  Q_INVOKABLE QJSValue useOn(QJSValue value) {
    auto convert = JSValueConverter::On(this);

    msg1.reset(new Message1);
    convert->fromJSValue(*msg1, value);
    // QCOMPARE(msg1->int_field1(), -42);
    qInfo() << "COMPARE -42 with" << msg1->int_field1();

    msg2.reset(new Message2);
    msg2->set_text_field2("This is \"on\" method test");
    return convert->toJSValue(*msg2);
  }

  Q_INVOKABLE QJSValue useFor(QJSValue value) {
    auto convert = JSValueConverter::For(value);

    msg1.reset(new Message1);
    convert->fromJSValue(*msg1, value);
    // QCOMPARE(msg1->int_field1(), 999999999);
    qInfo() << "COMPARE 999999999 with" << msg1->int_field1();

    msg2.reset(new Message2);
    msg2->set_text_field2("This is \"for\" method test");
    return convert->toJSValue(*msg2);
  }

  std::unique_ptr<Message1> msg1;
  std::unique_ptr<Message2> msg2;
};

class TestConvert : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void test1();

private:
  std::unique_ptr<QQmlApplicationEngine> engine;
};

void TestConvert::initTestCase() {
  engine.reset(new QQmlApplicationEngine);
  qmlRegisterType<SomeQmlType>("SomeModule", 1, 0, "SomeQmlType");
  engine->load("test_convert.qml");
}

void TestConvert::test1() {
  std::unique_ptr<QQuickView> view(new QQuickView(0));
  auto qml_path = QCoreApplication::applicationDirPath() + "/../test/protobuf/cpp/test_convert.qml";
  view->setSource(qml_path);
  view->show();
  auto test = view->rootObject()->findChild<QQuickItem*>("ConvertTest");
  QCOMPARE(test->property("testOnResult").toString(),
           QStringLiteral("This is \"on\" method test"));
  QCOMPARE(test->property("testForResult").toString(),
           QStringLiteral("This is \"for\" method test"));
}

QTEST_MAIN(TestConvert)
#include "test_convert.moc"
