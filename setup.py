from setuptools import setup, Extension

with open('requirements.txt', 'r') as reqfile:
    dependencies = reqfile.readlines()

test_deps = [
    'coverage',
    'pytest-cov'
]
extras = {
    'test': test_deps,
}

def main():
    setup(name="netgross",
          version="1.0.0",
          packages=["netgross"],
          description="Module for network computing",
          author="djanloo",
          author_email='becuzzigianluca@gmail.com',
          ext_modules=[Extension("cnets", ["netgross/cnets/cnets.c", "netgross/cnets/cutils.c"])],
          install_requires=dependencies,
          tests_require=test_deps,  # these two lines install stuff for
          extras_require=extras)    # test and coverage

if __name__ == "__main__":
    main()
