import os
import subprocess
from pathlib import Path

# Директории
AUX_LIBS_DIR = Path("../../aux_libs")  # Относительно расположения скрипта
AUX_LIBS_DIR.mkdir(parents=True, exist_ok=True)

# Репозитории для клонирования
REPOSITORIES = {
    "fmt": "https://github.com/fmtlib/fmt.git",
    "boost": "https://github.com/boostorg/boost.git",
    "openssl": "https://github.com/openssl/openssl.git",
    "libssh": "https://git.libssh.org/projects/libssh.git",
    "cryptopp": "https://github.com/weidai11/cryptopp.git",
    "zlib": "https://github.com/madler/zlib.git",
    "ziplib": "https://github.com/frk1/ziplib.git",
    "pugixml": "https://github.com/zeux/pugixml.git",
    "googletest": "https://github.com/google/googletest.git",
    "grpc": "https://github.com/grpc/grpc.git",
    "spdlog": "https://github.com/gabime/spdlog.git",
    "krb5": "https://github.com/krb5/krb5.git",
}

def clone_repository(repo_name, repo_url, target_dir):
    """Клонирует репозиторий в указанную директорию"""
    repo_dir = target_dir / repo_name
    
    if repo_dir.exists():
        print(f"Репозиторий {repo_name} уже существует в {repo_dir}, пропускаем...")
        return
    
    print(f"Клонирование {repo_name} из {repo_url}...")
    try:
        subprocess.run(["git", "clone", repo_url, str(repo_dir)], check=True)
        print(f"Успешно клонирован {repo_name}")
    except subprocess.CalledProcessError as e:
        print(f"Ошибка при клонировании {repo_name}: {e}")

def main():
    # Получаем абсолютный путь к директории aux_libs
    aux_libs_abs = (Path(__file__).parent / AUX_LIBS_DIR).resolve()
    
    print(f"Клонирование библиотек в директорию: {aux_libs_abs}")
    
    for repo_name, repo_url in REPOSITORIES.items():
        clone_repository(repo_name, repo_url, aux_libs_abs)

if __name__ == "__main__":
    main()