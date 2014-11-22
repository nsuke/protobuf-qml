#include <QObject>
#include <QtQml>
#include <QVariant>
#include <QList>
#include <QMap>
#include <QJSValue>

namespace qpb {

class Q_DECL_EXPORT Sandbox : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool hasError READ has_error NOTIFY hasErrorChanged)

signals:
  void hasErrorChanged();

 public:
  Sandbox(QObject* p = 0) : QObject(p) {}

  // causes SegFault
  Q_INVOKABLE QJSValue batchJSReturn(int v, int num) {
    auto engine = qmlEngine(this);
    auto acc = engine->newArray();
    for (int i = 0; i < num; i++) {
      acc.setProperty(i, v * v);
    }
    return acc;
  }

  Q_INVOKABLE QList<int> batchReturn(int v, int num) {
    QList<int> acc;
    for (int i = 0; i < num; i++) {
      acc.append(v * v);
    }
    return acc;
  }

  Q_INVOKABLE QVariantList batchListReturn(int v, int num) {
    QVariantList acc;
    for (int i = 0; i < num; i++) {
      acc.append(v * v);
    }
    return acc;
  }

  Q_INVOKABLE QVariantMap batchMapReturn(int v, int num) {
    QVariantMap acc;
    for (int i = 0; i < num; i++) {
      acc.insert("0", v * v);
    }
    return acc;
  }

  Q_INVOKABLE QVariantList listReturn(int i) {
    return QVariantList({i * i, false});
  }

  Q_INVOKABLE QVariantMap variantReturn(int i) {
    return QVariantMap({{"value", i * i}});
  }

  Q_INVOKABLE int propertyReturn(int i) {
    set_has_error(false);
    return i * i;
  }

  bool has_error() const { return has_error_; }

  void set_has_error(bool v) {
    if (has_error_ != v) {
      has_error_ = v;
      hasErrorChanged();
    }
  }

 private:
  bool has_error_ = false;
};
}
