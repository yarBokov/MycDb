import os
import sys
import subprocess
import argparse
from pathlib import Path
from typing import List, Optional

class BuildSystem:
    def __init__(self):
        self.script_dir = Path(__file__).parent.resolve()
        self.build_dir = self.script_dir / "build"
        self.default_build_type = "RelWithDebInfo"
        # self.cpack_generator = "RPM"
        
        self.build_type: Optional[str] = None
        self.enable_tests = False
        self.static_build = False
        self.libs_dir: Optional[str] = None
        self.cmake_extra_args: List[str] = []
        
        self._setup_arguments()

    def _setup_arguments(self):
        parser = argparse.ArgumentParser(
            description="Project build system controller",
            formatter_class=argparse.ArgumentDefaultsHelpFormatter
        )
        
        parser.add_argument("--build", 
                          choices=["Debug", "RelWithDebInfo"],
                          help="Set build configuration type")
        parser.add_argument("--tests", 
                          action="store_true",
                          help="Enable test compilation and execution")
        parser.add_argument("--static", 
                          action="store_true",
                          help="Build static libraries")
        parser.add_argument("--libs-dir", 
                          type=str,
                          help="Path to external libraries directory")
        # parser.add_argument("--cpack-generator", 
        #                   type=str,
        #                   default=self.cpack_generator,
        #                   help="CPack generator to use")
        parser.add_argument("--cmake-extra", 
                          nargs=argparse.REMAINDER,
                          help="Extra CMake arguments")
        
        args = parser.parse_args()
        
        self.build_type = args.build
        self.enable_tests = args.tests
        self.static_build = args.static
        self.libs_dir = args.libs_dir
        # self.cpack_generator = args.cpack_generator
        self.cmake_extra_args = args.cmake_extra or []

    def _validate_build_type(self):
        if self.build_type not in ["Debug", "RelWithDebInfo"]:
            msg = (f"Invalid build_type '{self.build_type}'. "
                  f"Default chosen: {self.default_build_type}")
            print(msg)
            self.build_type = self.default_build_type

    def _generate_cmake_args(self) -> List[str]:
        args = []
        
        # Build type configuration
        build_type = self.build_type or self.default_build_type
        args.extend(["-DCMAKE_BUILD_TYPE=" + build_type])
        
        # Test configuration
        args.append(f"-DTESTS={'1' if self.enable_tests else '0'}")
        
        # Static build
        args.append(f"-DSTATIC={'1' if self.static_build else '0'}")
        
        # Libraries directory
        if self.libs_dir:
            args.append(f"-DLIBS_DIR={self.libs_dir}")
        
        # CPack generator
        # args.append(f"-DCPACK_GENERATOR={self.cpack_generator}")
        
        # Extra arguments
        args.extend(self.cmake_extra_args)
        
        return args

    def _run_command(self, cmd: List[str], cwd: Optional[Path] = None):
        try:
            subprocess.run(cmd, check=True, cwd=cwd)
            return True
        except subprocess.CalledProcessError as e:
            print(f"Command failed: {' '.join(cmd)}", file=sys.stderr)
            return False

    def _run_tests(self):
        lib_dir = self.script_dir / "out" / "lib"
        env = os.environ.copy()
        env["LD_LIBRARY_PATH"] = f"{lib_dir}:{env.get('LD_LIBRARY_PATH', '')}"
        env["GTEST_COLOR"] = "0"
        
        print("Unit tests run started...")
        test_cmd = [
            "ctest",
            "--output-on-failure",
            "--output-junit", "testing/tmp/summary.xml",
            "-Q"
        ]
        
        try:
            subprocess.run(test_cmd, check=True, cwd=self.build_dir, env=env)
            print("Unit tests run finished!")
            return True
        except subprocess.CalledProcessError as e:
            print(f"Unit tests run FAILED with code {e.returncode}!")
            return False

    def execute(self):
        if self.build_type:
            self._validate_build_type()
        
        self.build_dir.mkdir(exist_ok=True)
        
        cmake_args = ["cmake", "-G", "Unix Makefiles"]
        cmake_args.extend(self._generate_cmake_args())
        cmake_args.append(str(self.script_dir))
        
        if not (self._run_command(cmake_args, self.build_dir) and 
            not self._run_command(["make", "-j", str(os.cpu_count())], self.build_dir)):
            sys.exit(1)
        
        if self.enable_tests and not self._run_tests():
            sys.exit(1)
        
        print("Build completed successfully!")

if __name__ == "__main__":
    builder = BuildSystem()
    builder.execute()