/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

package test.fixtures.complex_union;

import com.facebook.swift.codec.*;
import com.facebook.swift.codec.ThriftField.Requiredness;
import com.facebook.swift.codec.ThriftField.Recursiveness;
import java.util.*;

import static com.google.common.base.MoreObjects.toStringHelper;

@SwiftGenerated
@ThriftStruct("NonCopyableStruct")
public final class NonCopyableStruct {
    @ThriftConstructor
    public NonCopyableStruct(
        @ThriftField(value=1, name="num", requiredness=Requiredness.NONE) final long num
    ) {
        this.num = num;
    }
    
    protected NonCopyableStruct() {
      this.num = 0L;
    }
    
    public static class Builder {
        private long num;
    
        public Builder setNum(long num) {
            this.num = num;
            return this;
        }
    
        public Builder() { }
        public Builder(NonCopyableStruct other) {
            this.num = other.num;
        }
    
        public NonCopyableStruct build() {
            return new NonCopyableStruct (
                this.num
            );
        }
    }
    
    private final long num;

    
    @ThriftField(value=1, name="num", requiredness=Requiredness.NONE)
    public long getNum() { return num; }
    
    @Override
    public String toString() {
        return toStringHelper(this)
            .add("num", num)
            .toString();
    }
    
    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null || getClass() != o.getClass()) {
            return false;
        }
    
        NonCopyableStruct other = (NonCopyableStruct)o;
    
        return
            Objects.equals(num, other.num) &&
            true;
    }
    
    @Override
    public int hashCode() {
        return Arrays.deepHashCode(new Object[] {
            num
        });
    }
    
}
