"""
Race condition analyzer module
Module chuyên dụng để phân tích race conditions trong C++ code
"""
import os
import re
from typing import Dict, List
from lgedv.modules.config import get_cpp_dir, setup_logging
from lgedv.modules.data_models import SharedResource, ThreadUsage, RaceCondition, AnalysisResult
from lgedv.modules.file_utils import list_cpp_files

logger = setup_logging()

class CppParser:
    """Parser để phân tích C++ code"""
    
    def __init__(self):
        self.patterns = {
            # Global variable detection (chỉ ở file scope)
            'global_var': r'^\s*(?:extern\s+)?(?:const\s+)?(?:static\s+)?(?:const\s+)?([a-zA-Z_:][a-zA-Z0-9_:<>*&\s]*)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*(?:\[.*\])?\s*(?:=.*)?;',
            # Static variables
            'static_var': r'^\s*static\s+(?:const\s+)?([a-zA-Z_:][a-zA-Z0-9_:<>*&\s]*)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*(?:\[.*\])?\s*(?:=.*)?;',
            # Function/method detection
            'function_start': r'^\s*(?:.*\s+)?(\w+)\s*\([^)]*\)\s*(?:const\s*)?(?:override\s*)?(?:final\s*)?(?:noexcept\s*)?(?:\s*->\s*\w+\s*)?\s*\{',
            # Class detection
            'class_start': r'^\s*(?:class|struct)\s+(\w+)',
            # Namespace detection
            'namespace_start': r'^\s*namespace\s+(\w+)',
            # Thread creation patterns
            'thread_create': r'std::thread|pthread_create|std::async|CreateThread',
            # Mutex and synchronization
            'mutex_usage': r'std::mutex|std::recursive_mutex|pthread_mutex|std::shared_mutex|std::lock_guard|std::unique_lock',
            # Atomic operations
            'atomic_usage': r'std::atomic|atomic_|std::memory_order',
            # Smart pointers
            'shared_ptr': r'std::shared_ptr|std::unique_ptr|std::weak_ptr',
            # Singleton patterns
            'singleton': r'class\s+\w+.*singleton|getInstance\(\)|static\s+\w+\s*\*\s*instance'
        }
    
    def parse_file(self, file_path: str) -> Dict:
        """
        Parse C++ file để tìm shared resources và thread usage
        
        Args:
            file_path: Đường dẫn đến file C++
            
        Returns:
            Dict: Kết quả phân tích file
        """
        result = {
            "shared_resources": [],
            "thread_usage": [],
            "mutex_usage": [],
            "atomic_usage": []
        }
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
                lines = content.split('\n')
            
            # Context tracking để phân biệt global vs function scope
            brace_level = 0
            in_function = False
            in_class = False
            in_namespace = False
            
            for i, line in enumerate(lines, 1):
                stripped_line = line.strip()
                
                # Skip empty lines and comments
                if not stripped_line or stripped_line.startswith('//') or stripped_line.startswith('/*'):
                    continue
                
                # Count braces để track scope
                brace_level += stripped_line.count('{') - stripped_line.count('}')
                
                # Detect function/class/namespace starts
                if re.search(self.patterns['function_start'], stripped_line):
                    in_function = True
                elif re.search(self.patterns['class_start'], stripped_line):
                    in_class = True
                elif re.search(self.patterns['namespace_start'], stripped_line):
                    in_namespace = True
                
                # Reset scope flags when braces close
                if brace_level == 0:
                    in_function = False
                    in_class = False
                
                # Analyze patterns
                self._analyze_line(stripped_line, i, file_path, result, 
                                 in_function, in_class, in_namespace)
        
        except Exception as e:
            logger.error(f"Error parsing {file_path}: {e}")
        
        return result
    
    def _analyze_line(self, line: str, line_num: int, file_path: str, result: Dict,
                     in_function: bool, in_class: bool, in_namespace: bool):
        """Phân tích một dòng code"""
        
        # Global variables (chỉ khi không trong function/class)
        if not in_function and not in_class:
            if match := re.search(self.patterns['global_var'], line):
                var_type, var_name = match.groups()
                if not any(keyword in var_type.lower() for keyword in ['include', 'define', 'pragma']):
                    result["shared_resources"].append({
                        'file': file_path,
                        'line': line_num,
                        'type': 'global_var',
                        'name': var_name,
                        'var_type': var_type.strip(),
                        'scope': 'global'
                    })
        
        # Static variables
        if match := re.search(self.patterns['static_var'], line):
            var_type, var_name = match.groups()
            scope = 'function_static' if in_function else 'file_static'
            result["shared_resources"].append({
                'file': file_path,
                'line': line_num,
                'type': 'static_var',
                'name': var_name,
                'var_type': var_type.strip(),
                'scope': scope
            })
        
        # Thread usage
        if re.search(self.patterns['thread_create'], line, re.IGNORECASE):
            result["thread_usage"].append({
                'file': file_path,
                'line': line_num,
                'type': 'thread_creation',
                'code': line[:100]  # Limit length
            })
        
        # Mutex usage
        if re.search(self.patterns['mutex_usage'], line, re.IGNORECASE):
            result["mutex_usage"].append({
                'file': file_path,
                'line': line_num,
                'type': 'mutex_usage',
                'code': line[:100]
            })
        
        # Atomic usage
        if re.search(self.patterns['atomic_usage'], line, re.IGNORECASE):
            result["atomic_usage"].append({
                'file': file_path,
                'line': line_num,
                'type': 'atomic_usage',
                'code': line[:100]
            })

class RaceConditionAnalyzer:
    """Analyzer chính để phát hiện race conditions"""
    
    def __init__(self):
        self.parser = CppParser()
    
    def analyze_codebase(self, dir_path: str = None) -> AnalysisResult:
        """
        Phân tích race condition trong toàn bộ codebase
        
        Args:
            dir_path: Thư mục cần phân tích
            
        Returns:
            AnalysisResult: Kết quả phân tích
        """
        if dir_path is None:
            dir_path = get_cpp_dir()
        
        # Thu thập tất cả file C++
        cpp_files = list_cpp_files(dir_path)
        full_paths = [os.path.join(dir_path, f) for f in cpp_files]
        
        # Phân tích từng file
        all_resources = {}
        all_threads = {}
        
        for file_path in full_paths:
            analysis = self.parser.parse_file(file_path)
            all_resources[file_path] = analysis["shared_resources"]
            all_threads[file_path] = analysis["thread_usage"]
        
        # Tìm potential race conditions
        resource_access_map = self._build_resource_access_map(all_resources)
        potential_races = self._detect_race_conditions(resource_access_map, all_threads)
        
        # Tạo summary
        summary = {
            "total_files_analyzed": len(cpp_files),
            "total_shared_resources": len(resource_access_map),
            "potential_races": len(potential_races),
            "files_with_threads": len([f for f, threads in all_threads.items() if threads])
        }
        
        return AnalysisResult(
            shared_resources=resource_access_map,
            thread_usage=all_threads,
            potential_race_conditions=potential_races,
            summary=summary
        )
    
    def _build_resource_access_map(self, all_resources: Dict) -> Dict:
        """Xây dựng map của shared resources"""
        resource_access_map = {}
        
        for file_path, resources in all_resources.items():
            for resource in resources:
                resource_name = resource.get('name', 'unknown')
                if resource_name not in resource_access_map:
                    resource_access_map[resource_name] = []
                resource_access_map[resource_name].append(resource)
        
        return resource_access_map
    
    def _detect_race_conditions(self, resource_access_map: Dict, 
                               all_threads: Dict) -> List[RaceCondition]:
        """Phát hiện race conditions"""
        potential_races = []
        
        # Tìm shared resources được truy cập từ nhiều nơi
        for resource_name, accesses in resource_access_map.items():
            if len(accesses) > 1:
                # Check if there are threads in involved files
                files_involved = list(set(access['file'] for access in accesses))
                has_threads = any(all_threads.get(f, []) for f in files_involved)
                
                if has_threads:
                    severity = self._assess_severity(resource_name, accesses, all_threads)
                    
                    race_condition = RaceCondition(
                        severity=severity,
                        type="data_race",
                        description=f"Potential data race on shared resource '{resource_name}'",
                        files_involved=files_involved,
                        resources_involved=[resource_name],
                        line_numbers=[access['line'] for access in accesses],
                        fix_recommendation=f"Add proper synchronization (mutex/atomic) for '{resource_name}'"
                    )
                    potential_races.append(race_condition)
        
        return potential_races
    
    def _assess_severity(self, resource_name: str, accesses: List[Dict], 
                        all_threads: Dict) -> str:
        """Đánh giá mức độ nghiêm trọng của race condition"""
        # Simple severity assessment based on resource type and access pattern
        if any('global' in access.get('scope', '') for access in accesses):
            return "high"
        elif any('static' in access.get('type', '') for access in accesses):
            return "medium"
        else:
            return "low"

# Factory function
def analyze_race_conditions_in_codebase(dir_path: str = None) -> Dict:
    """
    Main entry point để phân tích race conditions
    
    Args:
        dir_path: Thư mục cần phân tích
        
    Returns:
        Dict: Kết quả phân tích dưới dạng dictionary
    """
    analyzer = RaceConditionAnalyzer()
    result = analyzer.analyze_codebase(dir_path)
    return result.to_dict()
