# distutils: language = c++
cdef class NothingSampler(TemporalSampler):
    def __cinit__(self, unsigned n):
        for i in range(n):
            self.vec.push_back(<unique_ptr[sampler_base]>unique_ptr[no_sampling](new no_sampling()))
    def get(self):
        return None
    
cdef class QtraitStatsSampler(TemporalSampler):
    def __cinit__(self, unsigned n, double optimum):
        for i in range(n):
            self.vec.push_back(<unique_ptr[sampler_base]>unique_ptr[pop_properties](new pop_properties(optimum)))
    def get(self):
        cdef vector[vector[qtrait_stats_cython]] rv
        cdef size_t i=0
        cdef size_t n=self.vec.size()
        while i<n:
            rv.push_back((<pop_properties*>(self.vec[i].get())).final())
            i+=1
        return rv

cdef class PopSampler(TemporalSampler):
    def __cinit__(self, unsigned n, unsigned nsam,GSLrng rng):
        for i in range(n):
            self.vec.push_back(<unique_ptr[sampler_base]>unique_ptr[sample_n](new sample_n(nsam,rng.thisptr.get())))
    def get(self):
        cdef vector[vector[pair[uint,detailed_deme_sample]]] rv
        cdef size_t i=0
        cdef size_t n=self.vec.size()
        while i<n:
            rv.push_back((<sample_n*>(self.vec[i].get())).final())
            i+=1
        return rv

cdef class VASampler(TemporalSampler):
    def __cinit__(self,unsigned n):
        for i in range(n):
            self.vec.push_back(<unique_ptr[sampler_base]>unique_ptr[additive_variance](new additive_variance()))
    def get(self):
        cdef vector[vector[VAcum]] rv
        cdef size_t i=0
        cdef size_t n=self.vec.size()
        while i<n:
            rv.push_back((<additive_variance*>(self.vec[i].get())).final())
            i+=1
        return rv

cdef class FreqSampler(TemporalSampler):
    def __cinit__(self,unsigned n):
        for i in range(n):
            self.vec.push_back(<unique_ptr[sampler_base]>unique_ptr[selected_mut_tracker](new selected_mut_tracker()))
    def get(self,unsigned minsojourn = 0, double minfreq = 0.0):
        cdef vector[vector[pair[selected_mut_data, vector[double]]]] rv
        cdef size_t i=0
        cdef size_t n=self.vec.size()
        while i<n:
            rv.push_back((<selected_mut_tracker*>self.vec[i].get()).final())
            i+=1
        return rv
