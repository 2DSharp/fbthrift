#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#
from libcpp.memory cimport shared_ptr
cimport thrift.py3.client


from module.clients_wrapper cimport cMyRootClientWrapper
from module.clients_wrapper cimport cMyNodeClientWrapper
from module.clients_wrapper cimport cMyLeafClientWrapper

cdef class MyRoot(thrift.py3.client.Client):
    cdef shared_ptr[cMyRootClientWrapper] _module_MyRoot_client

    cdef _module_MyRoot_set_client(MyRoot self, shared_ptr[cMyRootClientWrapper] c_obj)

    cdef _module_MyRoot_reset_client(MyRoot self)

cdef class MyNode(MyRoot):
    cdef shared_ptr[cMyNodeClientWrapper] _module_MyNode_client

    cdef _module_MyNode_set_client(MyNode self, shared_ptr[cMyNodeClientWrapper] c_obj)

    cdef _module_MyNode_reset_client(MyNode self)

cdef class MyLeaf(MyNode):
    cdef shared_ptr[cMyLeafClientWrapper] _module_MyLeaf_client

    cdef _module_MyLeaf_set_client(MyLeaf self, shared_ptr[cMyLeafClientWrapper] c_obj)

    cdef _module_MyLeaf_reset_client(MyLeaf self)

