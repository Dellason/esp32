import cv2
import numpy as np
import matplotlib.pyplot as plt
from typing import Tuple, Optional

class StereoTriangulation:
    def __init__(self, baseline: float, focal_length: float, image_width: int, image_height: int):
        """
        Initialize stereo triangulation system
        
        Args:
            baseline: Distance between the two cameras (in meters)
            focal_length: Focal length of cameras (in pixels)
            image_width: Width of camera images
            image_height: Height of camera images
        """
        self.baseline = baseline
        self.focal_length = focal_length
        self.image_width = image_width
        self.image_height = image_height
        
        # Camera matrices (assuming identical cameras)
        self.camera_matrix = np.array([
            [focal_length, 0, image_width/2],
            [0, focal_length, image_height/2],
            [0, 0, 1]
        ], dtype=np.float32)
        
        # Distortion coefficients (assuming minimal distortion)
        self.dist_coeffs = np.zeros((4, 1), dtype=np.float32)
        
        # Stereo rectification matrices
        self.R1 = self.R2 = np.eye(3, dtype=np.float32)
        self.P1 = self.P2 = None
        self.Q = None
        
        self._setup_stereo_rectification()
    
    def _setup_stereo_rectification(self):
        """Setup stereo rectification matrices"""
        # Rotation and translation between cameras
        R = np.eye(3, dtype=np.float32)  # No rotation (parallel cameras)
        T = np.array([[-self.baseline], [0], [0]], dtype=np.float32)
        
        # Stereo rectification
        self.R1, self.R2, self.P1, self.P2, self.Q = cv2.stereoRectify(
            self.camera_matrix, self.dist_coeffs,
            self.camera_matrix, self.dist_coeffs,
            (self.image_width, self.image_height),
            R, T,
            flags=cv2.CALIB_ZERO_DISPARITY,
            alpha=0
        )[:5]
    
    def rectify_images(self, left_img: np.ndarray, right_img: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """
        Rectify stereo image pair
        
        Args:
            left_img: Left camera image
            right_img: Right camera image
            
        Returns:
            Tuple of rectified (left, right) images
        """
        # Generate rectification maps
        map1_left, map2_left = cv2.initUndistortRectifyMap(
            self.camera_matrix, self.dist_coeffs, self.R1, self.P1,
            (self.image_width, self.image_height), cv2.CV_32FC1
        )
        
        map1_right, map2_right = cv2.initUndistortRectifyMap(
            self.camera_matrix, self.dist_coeffs, self.R2, self.P2,
            (self.image_width, self.image_height), cv2.CV_32FC1
        )
        
        # Apply rectification
        left_rectified = cv2.remap(left_img, map1_left, map2_left, cv2.INTER_LINEAR)
        right_rectified = cv2.remap(right_img, map1_right, map2_right, cv2.INTER_LINEAR)
        
        return left_rectified, right_rectified
    
    def compute_disparity(self, left_img: np.ndarray, right_img: np.ndarray) -> np.ndarray:
        """
        Compute disparity map using stereo block matching
        
        Args:
            left_img: Left rectified image
            right_img: Right rectified image
            
        Returns:
            Disparity map
        """
        # Convert to grayscale if needed
        if len(left_img.shape) == 3:
            left_gray = cv2.cvtColor(left_img, cv2.COLOR_BGR2GRAY)
            right_gray = cv2.cvtColor(right_img, cv2.COLOR_BGR2GRAY)
        else:
            left_gray = left_img
            right_gray = right_img
        
        # Create stereo matcher
        stereo = cv2.StereoBM_create(numDisparities=16*5, blockSize=15)
        
        # Compute disparity
        disparity = stereo.compute(left_gray, right_gray)
        
        # Convert to float and normalize
        disparity = disparity.astype(np.float32) / 16.0
        
        return disparity
    
    def disparity_to_depth(self, disparity: np.ndarray) -> np.ndarray:
        """
        Convert disparity map to depth map
        
        Args:
            disparity: Disparity map
            
        Returns:
            Depth map in meters
        """
        # Avoid division by zero
        valid_disparity = disparity > 0
        depth = np.zeros_like(disparity, dtype=np.float32)
        
        # Calculate depth: Z = (focal_length * baseline) / disparity
        depth[valid_disparity] = (self.focal_length * self.baseline) / disparity[valid_disparity]
        
        return depth
    
    def get_object_distance(self, left_img: np.ndarray, right_img: np.ndarray, 
                          object_point: Tuple[int, int]) -> Optional[float]:
        """
        Calculate distance to a specific object point
        
        Args:
            left_img: Left camera image
            right_img: Right camera image
            object_point: (x, y) coordinates of object in left image
            
        Returns:
            Distance to object in meters, or None if calculation fails
        """
        # Rectify images
        left_rect, right_rect = self.rectify_images(left_img, right_img)
        
        # Compute disparity
        disparity = self.compute_disparity(left_rect, right_rect)
        
        # Get disparity at object point
        x, y = object_point
        if 0 <= x < disparity.shape[1] and 0 <= y < disparity.shape[0]:
            disp_value = disparity[y, x]
            
            if disp_value > 0:
                # Calculate distance
                distance = (self.focal_length * self.baseline) / disp_value
                return distance
        
        return None
    
    def get_full_depth_map(self, left_img: np.ndarray, right_img: np.ndarray) -> np.ndarray:
        """
        Generate complete depth map from stereo pair
        
        Args:
            left_img: Left camera image
            right_img: Right camera image
            
        Returns:
            Depth map in meters
        """
        # Rectify images
        left_rect, right_rect = self.rectify_images(left_img, right_img)
        
        # Compute disparity
        disparity = self.compute_disparity(left_rect, right_rect)
        
        # Convert to depth
        depth_map = self.disparity_to_depth(disparity)
        
        return depth_map
    
    def visualize_results(self, left_img: np.ndarray, right_img: np.ndarray, 
                         object_point: Optional[Tuple[int, int]] = None):
        """
        Visualize stereo triangulation results
        
        Args:
            left_img: Left camera image
            right_img: Right camera image
            object_point: Optional point to highlight
        """
        # Rectify images
        left_rect, right_rect = self.rectify_images(left_img, right_img)
        
        # Compute disparity and depth
        disparity = self.compute_disparity(left_rect, right_rect)
        depth_map = self.disparity_to_depth(disparity)
        
        # Create visualization
        fig, axes = plt.subplots(2, 2, figsize=(12, 10))
        
        # Original images
        axes[0, 0].imshow(cv2.cvtColor(left_img, cv2.COLOR_BGR2RGB) if len(left_img.shape) == 3 else left_img, cmap='gray')
        axes[0, 0].set_title('Left Camera')
        axes[0, 0].axis('off')
        
        axes[0, 1].imshow(cv2.cvtColor(right_img, cv2.COLOR_BGR2RGB) if len(right_img.shape) == 3 else right_img, cmap='gray')
        axes[0, 1].set_title('Right Camera')
        axes[0, 1].axis('off')
        
        # Disparity map
        disp_vis = axes[1, 0].imshow(disparity, cmap='jet')
        axes[1, 0].set_title('Disparity Map')
        axes[1, 0].axis('off')
        plt.colorbar(disp_vis, ax=axes[1, 0])
        
        # Depth map
        depth_vis = axes[1, 1].imshow(depth_map, cmap='viridis', vmin=0, vmax=10)
        axes[1, 1].set_title('Depth Map (meters)')
        axes[1, 1].axis('off')
        plt.colorbar(depth_vis, ax=axes[1, 1])
        
        # Highlight object point if provided
        if object_point:
            x, y = object_point
            axes[0, 0].plot(x, y, 'ro', markersize=8)
            axes[1, 0].plot(x, y, 'ro', markersize=8)
            axes[1, 1].plot(x, y, 'ro', markersize=8)
            
            distance = self.get_object_distance(left_img, right_img, object_point)
            if distance:
                axes[0, 0].text(x+10, y, f'Distance: {distance:.2f}m', 
                               color='red', fontsize=10, weight='bold')
        
        plt.tight_layout()
        plt.show()


# Example usage and testing
def create_test_images():
    """Create synthetic test images for demonstration"""
    # Create simple test pattern
    height, width = 480, 640
    left_img = np.zeros((height, width, 3), dtype=np.uint8)
    right_img = np.zeros((height, width, 3), dtype=np.uint8)
    
    # Add some rectangles at different depths (simulated by horizontal shift)
    # Object 1: Close object (large disparity)
    cv2.rectangle(left_img, (100, 100), (200, 200), (255, 0, 0), -1)
    cv2.rectangle(right_img, (80, 100), (180, 200), (255, 0, 0), -1)  # 20 pixel shift
    
    # Object 2: Medium distance object
    cv2.rectangle(left_img, (300, 150), (400, 250), (0, 255, 0), -1)
    cv2.rectangle(right_img, (290, 150), (390, 250), (0, 255, 0), -1)  # 10 pixel shift
    
    # Object 3: Far object (small disparity)
    cv2.rectangle(left_img, (450, 200), (550, 300), (0, 0, 255), -1)
    cv2.rectangle(right_img, (445, 200), (545, 300), (0, 0, 255), -1)  # 5 pixel shift
    
    return left_img, right_img


# Example usage
if __name__ == "__main__":
    # Camera parameters (example values)
    baseline = 0.1  # 10 cm between cameras
    focal_length = 800  # focal length in pixels
    image_width = 640
    image_height = 480
    
    # Create stereo triangulation system
    stereo_system = StereoTriangulation(baseline, focal_length, image_width, image_height)
    
    # Create test images
    left_test, right_test = create_test_images()
    
    # Test distance calculation for specific points
    test_points = [(150, 150), (350, 200), (500, 250)]  # Points on our test objects
    
    print("Distance calculations:")
    for i, point in enumerate(test_points):
        distance = stereo_system.get_object_distance(left_test, right_test, point)
        if distance:
            print(f"Object {i+1} at point {point}: {distance:.2f} meters")
        else:
            print(f"Object {i+1} at point {point}: Unable to calculate distance")
    
    # Generate full depth map
    depth_map = stereo_system.get_full_depth_map(left_test, right_test)
    print(f"\nDepth map shape: {depth_map.shape}")
    print(f"Depth range: {np.min(depth_map[depth_map > 0]):.2f} - {np.max(depth_map):.2f} meters")
    
    # Visualize results (uncomment to see plots)
    # stereo_system.visualize_results(left_test, right_test, test_points[0])