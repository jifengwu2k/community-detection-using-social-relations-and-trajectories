from numpy import empty


class vector:
    def __init__(self, initial_shape, dtype):
        self.dtype = dtype
        if isinstance(initial_shape, int):
            self.ndarray_shape = (initial_shape,)
        else:
            assert isinstance(initial_shape, tuple)
            self.ndarray_shape = initial_shape
        
        # ndarray_shape and capacity must be positive
        self.capacity, *_ = self.ndarray_shape
        if self.capacity <= 0:
            self.capacity = 1
            self.ndarray_shape = (self.capacity, *self.ndarray_shape[1:])
        
        self.length = 0
        
        self.ndarray = empty(self.ndarray_shape, dtype=dtype)
        
    def append(self, x):
        if self.length < self.capacity:
            self.ndarray[self.length] = x
            self.length += 1
        else:
            self.capacity *= 2
            self.ndarray_shape = (self.capacity, *self.ndarray_shape[1:])
            
            new_ndarray = empty(self.ndarray_shape, dtype=self.dtype)
            new_ndarray[:self.length] = self.ndarray
            self.ndarray = new_ndarray
            
            self.ndarray[self.length] = x
            self.length += 1
    
    def __iter__(self):
        return iter(self.ndarray[:self.length])
    
    def __len__(self):
        return self.length
    
    def __getitem__(self, *args):
        return self.ndarray[:self.length].__getitem__(*args)

    def get(self):
        return self.ndarray[:self.length]

