import math

class Region(object):
    """
    Representation of a "region" in a simulation.

    Attributes:
        b: the beginning of the region
        
        e: the end of the region
        
        w: the "weight" assigned to the region

    See :func:`evolve_regions` for how this class may be used to parameterize a simulation
    """
    def __init__(self,float beg,float end,float weight,coupled=True):
        """
        Constructor

        :param beg: the beginning of the region
        :param end: the end of the region
        :param weight: the weight to assign
        :param coupled: if True, the weight is converted to (end-beg)*weight

        When coupled is True, the "weight" may be interpreted as a "per base pair"
        (or per unit, generally speaking) term.

        Example:

        >>> #A simple case
        >>> import fwdpy
        >>> r = fwdpy.Region(0,1,1)
        >>> #A more "biological" case:
        >>> #  The region covers positions 1 through 1,000,
        >>> #  and the per-base pair "weight" is 1e-5:
        >>> r = fwdpy.Region(1,1000,1e-5,True)
        """
        if math.isinf(beg):
            raise ValueError("fwdpy.Region: beg not finite")
        if math.isinf(end):
            raise ValueError("fwdpy.Region: end not finite")
        if math.isinf(weight):
            raise ValueError("fwdpy.Region: weight not finite")
        if math.isnan(beg):
            raise ValueError("fwdpy.Region: beg not a number")
        if math.isnan(end):
            raise ValueError("fwdpy.Region: end not a number")
        if math.isnan(weight):
            raise ValueError("fwdpy.Region: weight not a number")
        if weight < 0.0:
            raise ValueError("fwdpy.Region: weight < 0.0")
        self.b=float(beg)
        self.e=float(end)
        self.w=float(weight)
        self.c=coupled
        
class Sregion(Region):
    """
    Representation of a "region" in a simulation with a dominance term.

    This class is the base class for a general set of objects representing distributions of fitness effects.

    Attributes:
        b: the beginning of the region
        
        e: the end of the region
        
        w: the "weight" assigned to the region

        h: the dominance term

    See :func:`evolve_regions` for how this class may be used to parameterize a simulation
    """
    def __init__(self,float beg,float end,float weight,float h=1.0,coupled=True):
        """
        Constructor

        :param beg: the beginning of the region
        :param end: the end of the region
        :param weight: the weight to assign
        :param h: the dominance
        :param coupled: if True, the weight is converted to (end-beg)*weight

        When coupled is True, the "weight" may be interpreted as a "per base pair"
        (or per unit, generally speaking) term.

        Example:

        >>> #A simple case
        >>> import fwdpy
        >>> #Examples for models where the 3 genotype fitnesses are
        >>> #1, 1+sh, and 1+2s, respectively
        >>> recessive = fwdpy.Sregion(0,1,1,0)
        >>> additive = fwdpy.Sregion(0,1,1,1.0)
        >>> dominant = fwdpy.Sregion(0,1,1,2.0)
        """
        if math.isinf(h):
            raise ValueError("fwdpy.Sregion: h not finite")
        if math.isnan(h):
            raise ValueError("fwdpy.Segion: h not a number")
        self.h=float(h)
        super(Sregion,self).__init__(beg,end,weight,coupled)
        
class GammaS(Sregion):
    """
    Gamma distribution of fitness effects

    Attributes:
        b: the beginning of the region
        
        e: the end of the region
        
        w: the "weight" assigned to the region

        mean: mean of the Gamma

        shape: shape of the Gamma

        h: the dominance term

    See :func:`evolve_regions` for how this class may be used to parameterize a simulation
    """
    def __init__(self,float beg,float end,float weight,float mean,float shape,float h=1.0,coupled=True):
        """
        Constructor

        :param beg: the beginning of the region
        :param end: the end of the region
        :param weight: the weight to assign
        :param mean: the mean selection coefficient
        :param shape: the shape parameter of the distribution
        :param h: the dominance
        :param coupled: if True, the weight is converted to (end-beg)*weight

        When coupled is True, the "weight" may be interpreted as a "per base pair"
        (or per unit, generally speaking) term.

        Example:

        >>> #A simple case
        >>> import fwdpy
        >>> gdist = fwdpy.GammaS(0,1,1,-0.1,0.35)
        """
        if math.isinf(mean):
            raise ValueError("fwdpy.GammaS: mean not finite")
        if math.isinf(shape):
            raise ValueError("fwdpy.GammaS: shape not finite")
        if math.isnan(mean):
            raise ValueError("fwdpy.GammaS: mean not a number")
        if math.isnan(shape):
            raise ValueError("fwdpy.GammaS: shape not a number")
        self.mean=float(mean)
        self.shape=float(shape)
        super(GammaS,self).__init__(beg,end,weight,h,coupled)
        
class ConstantS(Sregion):
    """
    Constant/fixed selection coefficient

    Attributes:
        b: the beginning of the region
        
        e: the end of the region
        
        w: the "weight" assigned to the region

        s: the selection coefficient

        h: the dominance term

    See :func:`evolve_regions` for how this class may be used to parameterize a simulation
    """
    def __init__(self,float beg,float end,float weight,float s,float h=1.0,coupled=True):
        """
        Constructor

        :param beg: the beginning of the region
        :param end: the end of the region
        :param weight: the weight to assign
        :param s: the selection coefficient
        :param h: the dominance
        :param coupled: if True, the weight is converted to (end-beg)*weight

        When coupled is True, the "weight" may be interpreted as a "per base pair"
        (or per unit, generally speaking) term.

        Example:

        >>> #A simple case
        >>> import fwdpy
        >>> #s = -0.1 and h = 0
        >>> constantS = fwdpy.ConstantS(0,1,1,-0.1,0)
        """
        if math.isinf(s):
            raise ValueError("fwdpy.ConstantS: s not finite")
        if math.isnan(s):
            raise ValueError("fwdpy.ConstantS: s not a number")
        self.s=float(s)
        super(ConstantS,self).__init__(beg,end,weight,h,coupled)

class UniformS(Sregion):
    """
    Uniform distribution on selection coefficients

    Attributes:
        b: the beginning of the region
        
        e: the end of the region
        
        w: the "weight" assigned to the region

        lo: the lower bound on s

        hi: the upper bound on s
        
        h: the dominance term

    See :func:`evolve_regions` for how this class may be used to parameterize a simulation
    """
    def __init__(self,float beg,float end,float weight,float lo,float hi,float h=1.0,coupled=True):
        """
        Constructor
    
        :param beg: the beginning of the region
        :param end: the end of the region
        :param weight: the weight to assign
        :param lo: lower bound on s
        :param hi: upper bound on s
        :param h: the dominance
        :param coupled: if True, the weight is converted to (end-beg)*weight

        When coupled is True, the "weight" may be interpreted as a "per base pair"
        (or per unit, generally speaking) term.

        Example:

        >>> #A simple case
        >>> import fwdpy
        >>> #s is uniform on [0,-1]
        >>> constantS = fwdpy.UniformS(0,1,1,0,-1,0)
        """
        if math.isinf(lo):
            raise ValueError("fwdpy.UniformS: lo not finite")
        if math.isinf(hi):
            raise ValueError("fwdpy.UniformS: hi not finite")
        if math.isnan(lo):
            raise ValueError("fwdpy.UniformS: lo not a number")
        if math.isnan(hi):
            raise ValueError("fwdpy.UniformS: hi not a number")
        self.lo=float(lo)
        self.hi=float(hi)
        super(UniformS,self).__init__(beg,end,weight,h,coupled)

class ExpS(Sregion):
    """
    Exponential distribution on selection coefficients

    Attributes:
        b: the beginning of the region
        
        e: the end of the region
        
        w: the "weight" assigned to the region

        mean: the mean selection coefficient
        
        h: the dominance term

    See :func:`evolve_regions` for how this class may be used to parameterize a simulation
    """
    def __init__(self,float beg,float end,float weight,float mean,float h=1.0,coupled=True):
        """
        Constructor
    
        :param beg: the beginning of the region
        :param end: the end of the region
        :param weight: the weight to assign
        :param mean: the mean selection coefficient
        :param h: the dominance
        :param coupled: if True, the weight is converted to (end-beg)*weight

        When coupled is True, the "weight" may be interpreted as a "per base pair"
        (or per unit, generally speaking) term.

        Example:

        >>> #A simple case
        >>> import fwdpy
        >>> #s is exp(-0.1) and recessive
        >>> constantS = fwdpy.ExpS(0,1,1,0,-0.1,0)
        """
        if math.isinf(mean):
            raise ValueError("fwdp.ExpS: mean not finite")
        if math.isnan(mean):
            raise ValueError("fwdpy.ExpS: mean not a number")
        self.mean=float(mean)
        super(ExpS,self).__init__(beg,end,weight,h,coupled)

class GaussianS(Sregion):
    """
    Gaussian distribution on selection coefficients (effect sizes for sims of quantitative traits)

    Attributes:
        b: the beginning of the region
        
        e: the end of the region
        
        w: the "weight" assigned to the region

        sd: the standard deviation
        
        h: the dominance term

    The mean is zero.

    See :func:`evolve_regions` for how this class may be used to parameterize a simulation
    """
    def __init__(self,float beg,float end,float weight,float sd,float h=1.0,coupled=True):
        """
        Constructor
    
        :param beg: the beginning of the region
        :param end: the end of the region
        :param weight: the weight to assign
        :param mean: the mean selection coefficient
        :param h: the dominance
        :param coupled: if True, the weight is converted to (end-beg)*weight

        When coupled is True, the "weight" may be interpreted as a "per base pair"
        (or per unit, generally speaking) term.

        Example:

        >>> #A simple case
        >>> import fwdpy
        >>> #s N(0,0.1) and co-dominant
        >>> constantS = fwdpy.GaussianS(0,1,1,0,0.1,1)
        """
        if math.isinf(sd):
            raise ValueError("fwdpy.GaussianS: sd not finite")
        if math.isnan(sd):
            raise ValueError("fwdpy.GaussianS: sd not a number")
        self.sd=float(sd)
        super(GaussianS,self).__init__(beg,end,weight,h,coupled)
