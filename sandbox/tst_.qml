import QtQuick 2.3
import QtTest 1.0
import Protobuf.Sandbox 1.0 as Qpb

Item {

  Qpb.Sandbox {
    id: sandbox
  }

  TestCase {
    function test_() {
      for(var i = 0; i < 3; i++) {
        console.log('--- QML Instance ---');
        bench_(sandbox);
        console.log('--- Singleton Instance ---');
        bench_(Qpb.DefaultSandbox);
      }
    }

    function bench_(instance) {
      var num = 1024 * 1024;
      var result = [];
      console.time("JS-init");
      var acc = 0;
      var v = 42;
      for(var i = 0; i < num; i++) {
        result[i] = v * v;
      }
      console.timeEnd("JS-init");
      console.time("JS");
      var acc = 0;
      var v = 42;
      for(var i = 0; i < num; i++) {
        result[i] = v * v;
      }
      console.timeEnd("JS");

      console.time("BatchReturn");
      result = instance.batchReturn(v, num);
      console.timeEnd("BatchReturn");

      console.time("BatchListReturn");
      result = instance.batchListReturn(v, num);
      console.timeEnd("BatchListReturn");

      console.time("BatchMapReturn");
      result = instance.batchMapReturn(v, num);
      console.timeEnd("BatchMapReturn");

      // causes SegFault
      // console.time("BatchJSReturn");
      // result = instance.batchJSReturn(v, num);
      // console.timeEnd("BatchJSReturn");

      result = [];
      console.time("VariantListReturn");
      for(var i = 0; i < num; i++) {
        var r = instance.listReturn(v);
        if(!r[1]) {
          result[i] = r[0];
        } else {
          console.error('fail');
        }
      }
      console.timeEnd("VariantListReturn");

      result = [];
      console.time("PropertyReturn");
      for(var i = 0; i < num; i++) {
        if(!instance.hasError) {
          result[i] = instance.propertyReturn(v);
        } else {
          console.error('fail');
        }
      }
      console.timeEnd("PropertyReturn");

      console.time("VariantMapReturn");
      for(var i = 0; i < num; i++) {
        var r = instance.variantReturn(v);
        if(!r.error) {
          result[i] = r.value;
        } else {
          console.error('fail');
        }
      }
      console.timeEnd("VariantMapReturn");
    }

  }
}

