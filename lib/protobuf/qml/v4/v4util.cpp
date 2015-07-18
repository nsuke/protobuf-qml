#include "protobuf/qml/v4/v4util.h"

#include <private/qqmlcontextwrapper_p.h>

using namespace QV4;

namespace protobuf {
namespace qml {

ReturnedValue packCallbackObject(ExecutionEngine* v4, const Value& callback) {
  Scope scope(v4);
  ScopedObject o(scope, v4->newObject());
  ScopedValue ctx(scope, QmlContextWrapper::qmlScope(
                             scope.engine, v4->v8Engine->callingContext(), 0));
  o->put(ScopedString(scope, v4->newString(QStringLiteral("CallingContext"))),
         ctx);
  ScopedFunctionObject cb(scope, callback);
  o->put(ScopedString(scope, v4->newString(QStringLiteral("CallbackObject"))),
         cb);
  return o->asReturnedValue();
}

std::pair<ReturnedValue, ReturnedValue> unpackCallbackObject(
    ExecutionEngine* v4, ReturnedValue packed) {
#define UNPACK_EMPTY_RESULT std::make_pair(Encode::null(), Encode::null())
  Scope scope(v4);
  ScopedObject o(scope, packed);
  if (!o) {
    // v4->throwError(QStringLiteral("No callback"));
    return UNPACK_EMPTY_RESULT;
  }
  ScopedString s(scope, v4->newString(QStringLiteral("CallingContext")));
  ScopedObject ctx(scope, o->get(s));
  if (!ctx) {
    qDebug() << "Calling context is no longer available";
    return UNPACK_EMPTY_RESULT;
  }
  s = v4->newString(QStringLiteral("CallbackObject"));
  ScopedObject cb(scope, o->get(s));
  if (!cb) {
    qDebug() << "No callback";
    return UNPACK_EMPTY_RESULT;
  }
  auto contextData = QmlContextWrapper::getContext(ctx);
  if (!contextData) {
    qDebug() << "Calling context is no longer available";
    return UNPACK_EMPTY_RESULT;
  }
  return std::make_pair(ctx.asReturnedValue(), cb.asReturnedValue());
#undef UNPACK_EMPTY_RESULT
}
}
}
