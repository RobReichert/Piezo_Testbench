import numpy as np

class NumpyFloatToFixConverter(object):
    """*** IMPORTED FROM RIG LIBRARY, RIG NOT AVAILABLE THROUGH CONDA CHANNELS***
    A callable which converts Numpy arrays of floats to fixed point arrays.
    General usage is to create a new converter and then call this on arrays of
    values.  The `dtype` of the returned array is determined from the
    parameters passed.  For example::
        >>> f = NumpyFloatToFixConverter(signed=True, n_bits=8, n_frac=4)
    Will convert floating point values to 8-bit signed representations with 4
    fractional bits.  Consequently the returned `dtype` will be `int8`::
        >>> import numpy as np
        >>> vals = np.array([0.0, 0.25, 0.5, -0.5, -0.25])
        >>> f(vals)
        array([ 0,  4,  8, -8, -4], dtype=int8)
    The conversion is saturating::
        >>> f(np.array([15.0, 16.0, -16.0, -17.0]))
        array([ 127,  127, -128, -128], dtype=int8)
    The byte representation can be expected to match that for using
    `float_to_fix`::
        >>> d = f(np.array([-16.0]))
        >>> import struct
        >>> g = float_to_fix(True, 8, 4)
        >>> val = g(-16.0)
        >>> struct.pack('B', val) == bytes(d.data)
        True
    An exception is raised if the number of bits specified cannot be
    represented using a whole `dtype`::
        >>> NumpyFloatToFixConverter(True, 12, 0)
        Traceback (most recent call last):
        ValueError: n_bits: 12: Must be 8, 16, 32 or 64.
    """
    dtypes = {
        (False, 8): np.uint8,
        (True, 8): np.int8,
        (False, 16): np.uint16,
        (True, 16): np.int16, 
        (False, 32): np.uint32,
        (True, 32): np.int32,
        (False, 64): np.uint64,
        (True, 64): np.int64,
    }

    def __init__(self, signed, n_bits, n_frac):
        """Create a new converter from floats into ints.
        Parameters
        ----------
        signed : bool
            Indicates that the converted values are to be signed or otherwise.
        n_bits : int
            The number of bits each value will use overall (must be 8, 16, 32,
            or 64).
        n_frac : int
            The number of fractional bits.
        """
        # Check the number of bits is sane
        if n_bits not in [8, 16, 32, 64]:
            raise ValueError(
                "n_bits: {}: Must be 8, 16, 32 or 64.".format(n_bits))

        # Determine the maximum and minimum values after conversion
        if signed:
            self.max_value = 2**(n_bits - 1) - 1
            self.min_value = -self.max_value - 1
        else:
            self.max_value = 2**n_bits - 1
            self.min_value = 0

        # Store the settings
        self.bytes_per_element = n_bits / 8
        self.dtype = self.dtypes[(signed, n_bits)]
        self.n_frac = n_frac

    def __call__(self, values):
        """Convert the given NumPy array of values into fixed point format."""
        # Scale and cast to appropriate int types
        vals = values * 2.0 ** self.n_frac

        # Saturate the values
        vals = np.clip(vals, self.min_value, self.max_value)

        # **NOTE** for some reason just casting resulted in shape
        # being zeroed on some indeterminate selection of OSes,
        # architectures, Python and Numpy versions"
        return np.array(vals, copy=True, dtype=self.dtype)