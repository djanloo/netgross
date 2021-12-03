"""Utility module to extend the getattr() method over iterables of class instances.

author: djanloo
date: 3 dic 2021

I am not aware of any built-in neither third-party way to do this,
so I did it by myself.

This module basically let this

    A = citer([inst1, inst2, inst3])
    print(A.attr)

    #prints [int1.attr, inst2.attr, inst3.attr]

become possible
"""


class citer:

    def __init__(self, objs = None):
        self.objs = objs if objs is not None else self.empty
        self._type = None
        self.citer_name = None

        if objs:
            for obj in objs:
                self.type = type(obj)
    @property
    def type(self):
        return self._type

    @type.setter
    def type(self, set_type):
        if self._type is None or set_type == self._type:
            self._type = set_type
        else:
            raise TypeError(f"clist type is already set to {self._type}")

    def __getattr__(self, attr_name):
        return [ getattr(obj, attr_name) for obj in self.objs]

    def __str__(self):
        desc = self.__class__.citer_name + "["
        for obj in self.objs:
            desc += f" {str(obj)} "
        desc += "]"
        return desc 

class clist(citer):

    citer_name = 'clist'

    def __init__(self, objs=None):
        self.empty = list()
        super().__init__(objs)

    def __iadd__(self, element):
        self.objs.append(element)
        return self
        

class cdict(citer):

    citer_name = 'cdict'

    def __init__(self, objs=None):
        self.empty = dict()
        super().__init__(objs)

    def __getattr__(self, attr_name):
        return [ getattr(obj, attr_name) for obj in self.objs.values()]
    
    def __iadd__(self, element):
        for key, value in element.items():
            self.objs[key] = value
        return self

class cset(citer):

    citer_name = 'cset'

    def __init__(self, objs=None):
        self.empty = set()
        super().__init__(objs)
    
    def __iadd__(self, element):
        self.objs.add(element)
        return self