/*
 * Copyright (c) 2016, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#ifndef SHARE_VM_AOT_AOTCODEHEAP_HPP
#define SHARE_VM_AOT_AOTCODEHEAP_HPP

#include "aot/aotCompiledMethod.hpp"
#include "classfile/symbolTable.hpp"
#include "oops/metadata.hpp"
#include "oops/method.hpp"

enum CodeState {
  not_set = 0, // _aot fields is not set yet
  in_use  = 1, // _aot field is set to corresponding AOTCompiledMethod
  invalid = 2  // AOT code is invalidated because dependencies failed
};

typedef struct {
  AOTCompiledMethod* _aot;
  CodeState _state; // State change cases: not_set->in_use, not_set->invalid
} CodeToAMethod;

class ClassLoaderData;

class AOTClass {
public:
  ClassLoaderData* _classloader;
};

typedef struct {
  int _name_offset;
  int _code_offset;
  int _meta_offset;
  int _metadata_got_offset;
  int _metadata_got_size;
  int _code_id;
} AOTMethodOffsets;

typedef struct {
  const char* _name;
  address     _code;
  aot_metadata* _meta;
  jlong*      _state_adr;
  address     _metadata_table;
  int         _metadata_size;
} AOTMethodData;

typedef struct {
  int _got_index;
  int _class_id;
  int _compiled_methods_offset;
  int _dependent_methods_offset;
  uint64_t _fingerprint;
} AOTKlassData;

typedef struct {
  int _version;
  int _class_count;
  int _method_count;
  int _metaspace_got_size;
  int _metadata_got_size;
  int _oop_got_size;
  int _jvm_version_offset;

  enum {
    AOT_SHARED_VERSION = 1
  };
} AOTHeader;

typedef struct {
  enum { CONFIG_SIZE = 12 + 7 * 4 };
  int _config_size;
  int _narrowOopShift;
  int _narrowKlassShift;
  int _contendedPaddingWidth;
  int _fieldsAllocationStyle;
  int _objectAlignment;
  int _codeSegmentSize;
  // byte[11] array map to boolean values here
  bool _debug_VM;
  bool _useCompressedOops;
  bool _useCompressedClassPointers;
  bool _compactFields;
  bool _useG1GC;
  bool _useCMSGC;
  bool _useTLAB;
  bool _useBiasedLocking;
  bool _tieredAOT;
  bool _enableContended;
  bool _restrictContended;
  bool _omitAssertions;
} AOTConfiguration;

class AOTLib : public CHeapObj<mtCode> {
  static bool _narrow_oop_shift_initialized;
  static int _narrow_oop_shift;
  static int _narrow_klass_shift;

  bool _valid;
  void* _dl_handle;
  const int _dso_id;
  const char* _name;
  // VM configuration during AOT compilation
  AOTConfiguration* _config;
  AOTHeader* _header;

  void handle_config_error(const char* format, ...) ATTRIBUTE_PRINTF(2, 3);
public:
  AOTLib(void* handle, const char* name, int dso_id);
  virtual ~AOTLib();
  static int  narrow_oop_shift() { return _narrow_oop_shift; }
  static int  narrow_klass_shift() { return _narrow_klass_shift; }
  static bool narrow_oop_shift_initialized() { return _narrow_oop_shift_initialized; }

  bool is_valid() const {
    return _valid;
  }
  const char* name() const {
    return _name;
  }
  void* dl_handle() const {
    return _dl_handle;
  }
  int id() const {
    return _dso_id;
  }
  AOTHeader* header() const {
    return _header;
  }
  AOTConfiguration* config() const {
    return _config;
  }
  void verify_config();
  void verify_flag(bool aot_flag, bool flag, const char* name);
  void verify_flag(int  aot_flag, int  flag, const char* name);

  address load_symbol(const char *name);
};


class AOTCodeHeap : public CodeHeap {
  AOTLib* _lib;
  int _aot_id;

  int _class_count;
  int _method_count;
  AOTClass* _classes;
  CodeToAMethod* _code_to_aot;

  address _code_space;
  address _code_segments;
  jlong*  _method_state;


  // Collect metaspace info: names -> address in .got section
  const char* _metaspace_names;
  address _method_metadata;

  address _methods_offsets;
  address _klasses_offsets;
  address _dependencies;

  Metadata** _metaspace_got;
  Metadata** _metadata_got;
  oop*    _oop_got;

  int _metaspace_got_size;
  int _metadata_got_size;
  int _oop_got_size;

  // Collect stubs info
  int* _stubs_offsets;

  address _low_boundary;

  bool _lib_symbols_initialized;

  void adjust_boundaries(AOTCompiledMethod* method) {
    address low = _low_boundary;
    if (method->code_begin() < low) {
      low = method->code_begin();
    }
    address high = high_boundary();
    if (method->code_end() > high) {
      high = method->code_end();
    }
    assert(_method_count > 0, "methods count should be set already");

    _low_boundary = low;
    _memory.set_high_boundary((char *)high);
    _memory.set_high((char *)high);
  }

  void register_stubs();

  void link_shared_runtime_symbols();
  void link_stub_routines_symbols();
  void link_os_symbols();
  void link_graal_runtime_symbols();

  void link_global_lib_symbols();
  void link_primitive_array_klasses();
  void publish_aot(const methodHandle& mh, AOTMethodData* method_data, int code_id);


  AOTCompiledMethod* next_in_use_at(int index) const;

  // Find klass in SystemDictionary for aot metadata.
  static Klass* lookup_klass(const char* name, int len, const Method* method, Thread* THREAD);
public:
  AOTCodeHeap(AOTLib* lib);
  virtual ~AOTCodeHeap();

  address low_boundary()  const { return _low_boundary; }
  address high_boundary() const { return (address)CodeHeap::high(); }

  bool contains(const void* p) const {
    bool result = (low_boundary() <= p) && (p < high_boundary());
    assert(!result || (_method_count > 0), "");
    assert(result == CodeHeap::contains(p), "");
    return result;
  }

  bool contains_blob(const CodeBlob* blob) const {
    return CodeHeap::contains(blob->code_begin());
  }

  AOTCompiledMethod* find_aot(address p) const;

  virtual void* find_start(void* p)     const;
  virtual CodeBlob* find_blob_unsafe(void* start) const;
  virtual void* first() const;
  virtual void* next(void *p) const;

  AOTKlassData* find_klass(InstanceKlass* ik);
  bool load_klass_data(instanceKlassHandle kh, Thread* thread);
  Klass* get_klass_from_got(const char* klass_name, int klass_len, const Method* method);
  void sweep_dependent_methods(AOTKlassData* klass_data);
  bool is_dependent_method(Klass* dependee, AOTCompiledMethod* aot);

  const char* get_name_at(int offset) {
    return _metaspace_names + offset;
  }

  void oops_do(OopClosure* f);
  void metadata_do(void f(Metadata*));
  void got_metadata_do(void f(Metadata*));

#ifdef ASSERT
  bool got_contains(Metadata **p) {
    return (p >= &_metadata_got[0] && p < &_metadata_got[_metadata_got_size]) ||
           (p >= &_metaspace_got[0] && p < &_metaspace_got[_metaspace_got_size]);
  }
#endif

  int dso_id() const { return _lib->id(); }
  int aot_id() const { return _aot_id; }

  int method_count() { return _method_count; }

  AOTCompiledMethod* get_code_desc_at_index(int index) {
    if (index < _method_count && _code_to_aot[index]._state == in_use) {
        AOTCompiledMethod* m = _code_to_aot[index]._aot;
        assert(m != NULL, "AOT method should be set");
        if (!m->is_runtime_stub()) {
          return m;
        }
    }
    return NULL;
  }

  static Method* find_method(KlassHandle klass, Thread* thread, const char* method_name);

  void cleanup_inline_caches();

  DEBUG_ONLY( int verify_icholder_relocations(); )

  void flush_evol_dependents_on(instanceKlassHandle dependee);

  void alive_methods_do(void f(CompiledMethod* nm));

#ifndef PRODUCT
  static int klasses_seen;
  static int aot_klasses_found;
  static int aot_klasses_fp_miss;
  static int aot_klasses_cl_miss;
  static int aot_methods_found;

  static void print_statistics();
#endif
};

#endif // SHARE_VM_AOT_AOTCODEHEAP_HPP
