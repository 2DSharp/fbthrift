#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#
from libcpp.memory cimport shared_ptr
cimport thrift.py3.client


from empty.clients_wrapper cimport cNullServiceClientWrapper

cdef class NullService(thrift.py3.client.Client):
    cdef shared_ptr[cNullServiceClientWrapper] _empty_NullService_client

    cdef _empty_NullService_set_client(NullService self, shared_ptr[cNullServiceClientWrapper] c_obj)

    cdef _empty_NullService_reset_client(NullService self)

