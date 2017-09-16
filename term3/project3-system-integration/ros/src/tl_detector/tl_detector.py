#!/usr/bin/env python
import rospy
from std_msgs.msg import Int32
from geometry_msgs.msg import PoseStamped, Pose
from styx_msgs.msg import TrafficLightArray, TrafficLight
from styx_msgs.msg import Lane
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
from light_classification.tl_classifier import TLClassifier
import tf
import cv2
from traffic_light_config import config

import numpy as np
from matplotlib import pyplot as plt

STATE_COUNT_THRESHOLD = 3


class TLDetector(object):
    def __init__(self):
        rospy.init_node('tl_detector')

        self.pose = None
        self.waypoints = None
        self.camera_image = None
        self.cv_image = None
        self.has_image = False
        self.lights = []

        self.prev_light_loc = None

        self.traffic_light_info = None

        sub1 = rospy.Subscriber('/current_pose', PoseStamped, self.pose_cb)
        sub2 = rospy.Subscriber('/base_waypoints', Lane, self.waypoints_cb)

        '''
        /vehicle/traffic_lights helps you acquire an accurate ground truth data source for the traffic light
        classifier, providing the location and current color state of all traffic lights in the
        simulator. This state can be used to generate classified images or subbed into your solution to
        help you work on another single component of the node. This topic won't be available when
        testing your solution in real life so don't rely on it in the final submission.
        '''
        sub3 = rospy.Subscriber('/vehicle/traffic_lights', TrafficLightArray, self.traffic_cb)
        sub4 = rospy.Subscriber('/camera/image_raw', Image, self.image_cb)

        self.upcoming_red_light_pub = rospy.Publisher('/traffic_waypoint', Int32, queue_size=1)

        self.bridge = CvBridge()
        self.light_classifier = TLClassifier()
        self.listener = tf.TransformListener()

        self.state = TrafficLight.UNKNOWN
        self.last_state = TrafficLight.UNKNOWN
        self.last_wp = -1
        self.state_count = 0

        light_positions = config.light_positions
        self.traffic_light_config_converter(light_positions)

        rospy.spin()

    def pose_cb(self, msg):
        self.pose = msg

    def waypoints_cb(self, msg):
        # rosmsg show styx_msgs/Lane
        self.waypoints = msg

        # print '######################'
        # print msg.waypoints[0].pose.pose.position.x
        # print msg.waypoints[0].pose.pose.position.y
        # print msg.waypoints[0].pose.pose.position.z
        # print '######################'

    def traffic_cb(self, msg):
        self.lights = msg.lights

    def image_cb(self, msg):
        """Identifies red lights in the incoming camera image and publishes the index
            of the waypoint closest to the red light to /traffic_waypoint

        Args:
            msg (Image): image from car-mounted camera

        """
        self.has_image = True
        self.camera_image = msg

        # visualize image for debugging purpose
        self.camera_image.encoding = "rgb8"
        self.cv_image = self.bridge.imgmsg_to_cv2(self.camera_image, "bgr8")
        # cv2.imshow("Camera", self.cv_image)
        # cv2.waitKey(1)

        light_wp, state = self.process_traffic_lights()

        '''
        Publish upcoming red lights at camera frequency.
        Each predicted state has to occur `STATE_COUNT_THRESHOLD` number
        of times till we start using it. Otherwise the previous stable state is
        used.
        '''
        if self.state != state:
            self.state_count = 0
            self.state = state
        elif self.state_count >= STATE_COUNT_THRESHOLD:
            self.last_state = self.state
            light_wp = light_wp if state == TrafficLight.RED else -1
            self.last_wp = light_wp
        # self.upcoming_red_light_pub.publish(Int32(light_wp))
        # else:
        #     self.upcoming_red_light_pub.publish(Int32(self.last_wp))
        self.state_count += 1

    def get_closest_waypoint(self, pose):
        """Identifies the closest path waypoint to the given position
            https://en.wikipedia.org/wiki/Closest_pair_of_points_problem
        Args:
            pose (Pose): position to match a waypoint to

        Returns:
            int: index of the closest waypoint in self.waypoints

        """
        # TODO: Think about orientation in front

        search_distance = 5.0
        vehicle_x = pose.position.x
        vehicle_y = pose.position.y

        min_distance = 9999
        min_index = None

        for i, waypoint in enumerate(self.waypoints.waypoints):
            wp_x = waypoint.pose.pose.position.x
            wp_y = waypoint.pose.pose.position.y
            dx = wp_x - vehicle_x
            dy = wp_y - vehicle_y
            distance = (dx ** 2 + dy ** 2) ** 0.5
            if distance > search_distance:
                continue

            if distance < min_distance:
                min_distance = distance
                min_index = i

        return min_index

    def get_closest_traffic_light(self, car_position_index):
        min_det_index = 9999
        min_index = 9999

        for i, traffic_light in enumerate(self.traffic_light_info.lights):
            index = self.get_closest_waypoint(traffic_light.pose.pose)
            det_index = index - car_position_index
            if det_index < 0:
                continue
            if det_index < min_det_index:
                min_det_index = det_index
                min_index = i

        # print 'closest_traffic_light = ' + str(min_index)
        return min_index

    def project_to_image_plane(self, point_in_world):
        """Project point from 3D world coordinates to 2D camera image location

        Args:
            point_in_world (Point): 3D location of a point in the world

        Returns:
            x (int): x coordinate of target point in image
            y (int): y coordinate of target point in image

        """

        # print point_in_world.x
        # print point_in_world.y
        # print point_in_world.z

        fx = config.camera_info.focal_length_x
        fy = config.camera_info.focal_length_y

        image_width = config.camera_info.image_width
        image_height = config.camera_info.image_height

        # get transform between pose of camera and world frame
        trans = None
        rot = None
        try:
            now = rospy.Time.now()
            self.listener.waitForTransform("/base_link", "/world", now, rospy.Duration(1.0))
            (trans, rot) = self.listener.lookupTransform("/base_link", "/world", now)

        except (tf.Exception, tf.LookupException, tf.ConnectivityException):
            rospy.logerr("Failed to find camera to map transform")

        # TODO Use tranform and rotation to calculate 2D position of light in image

        # print trans
        # print rot

        # x = 0
        # y = 0

        world_coordinates_point = np.array([point_in_world.x, point_in_world.y, point_in_world.z], dtype=np.float32).reshape(3, 1)

        car_position = np.array([self.pose.pose.position.x, self.pose.pose.position.y, self.pose.pose.position.z], dtype=np.float32).reshape(3, 1)
        camera_offset = np.array([1.0, 0, 1.2], dtype=np.float32).reshape(3, 1)
        translation_vector = car_position + camera_offset

        # Move point to camera origin
        world_coordinates_point_shifted_to_camera_coordinates = world_coordinates_point - translation_vector

        homogenous_vector = np.ones(shape=(4, 1), dtype=np.float32)
        homogenous_vector[:3] = world_coordinates_point_shifted_to_camera_coordinates

        quaternion = np.array([self.pose.pose.orientation.x,
                               self.pose.pose.orientation.y,
                               self.pose.pose.orientation.z,
                               self.pose.pose.orientation.w], dtype=np.float32)

        euler_angles = tf.transformations.euler_from_quaternion(quaternion)
        rotation_matrix = tf.transformations.euler_matrix(*euler_angles)

        point_in_camera_coordinates = np.dot(rotation_matrix, homogenous_vector)

        x = int((fx * point_in_camera_coordinates[0] * point_in_camera_coordinates[2]) + (image_width / 2))
        y = int((fy * point_in_camera_coordinates[1] * point_in_camera_coordinates[2]) + (image_height / 2))

        print (x, y)

        x_offset = 150
        y_offset = 100

        x_tl = x - x_offset
        y_tl = y - y_offset
        x_br = x + x_offset
        y_br = y + y_offset

        if x_tl < 0 or x_br > image_width or y_tl < 0 or y_br > image_height:
            return -1, -1

        cv_image2 = self.cv_image.copy()
        # cv2.circle(cv_image2, (x, y), 10, (0, 128, 128), -1)
        # cv2.rectangle(cv_image2, (x-100, y-50), (x+100, y+50),  (0, 0, 255), 3, -1)

        # region of interest
        roi = cv_image2[y_tl: y_br, x_tl:x_br]

        if roi is not None:
            roi_gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)

        circles = cv2.HoughCircles(roi_gray, cv2.HOUGH_GRADIENT, 1, 20, param1=50, param2=30, minRadius=0, maxRadius=0)

        if circles is not None:
            #print "Circle Num = " + str(len(circles))
            circles = np.uint16(np.around(circles))

            for i in circles[0, :]:
                # draw the outer circle
                cv2.circle(roi, (i[0], i[1]), i[2], (0, 255, 0), 2)
                # draw the center of the circle
                #cv2.circle(roi, (i[0], i[1]), 2, (0, 0, 255), 3)

                pt = roi[i[1], i[0]]

                print pt

                # bgr
                if pt[1] >= 200 and pt[2] >=200:
                    print "YELLOW!!!!"
                    cv2.putText(cv_image2, 'YELLOW', (10, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
                    continue
                if pt[2] >= 200:
                    print "RED!!!!"
                    cv2.putText(cv_image2, 'RED', (10, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
                    continue
                if pt[1] >= 200:
                    print "GREEN!!!!"
                    cv2.putText(cv_image2, 'GREEN', (10, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
                    continue



        # color = ('b', 'g', 'r')
        # for i, col in enumerate(color):
        #     histr = cv2.calcHist([roi], [i], None, [256], [0, 256])
        #
        #
        #
        #     plt.plot(histr, color=col)
        #     plt.xlim([0, 256])
        #
        # plt.show()

        cv2.imshow('Projection', cv_image2)
        cv2.imshow('ROI', roi)
        cv2.waitKey(1)


        return x, y

    def get_light_state(self, light):
        """Determines the current color of the traffic light

        Args:
            light (TrafficLight): light to classify

        Returns:
            int: ID of traffic light color (specified in styx_msgs/TrafficLight)

        """
        if not self.has_image:
            self.prev_light_loc = None
            return False

        x, y = self.project_to_image_plane(light.pose.pose.position)

        # TODO use light location to zoom in on traffic light in image

        # Get classification
        return self.light_classifier.get_classification(self.cv_image)

    def traffic_light_config_converter(self, light_positions):
        """Convert traffic light information from configuration file to 
            ros message TrafficLight
            
        """
        self.traffic_light_info = TrafficLightArray()
        for i, pos in enumerate(light_positions):
            traffic_light = TrafficLight()
            traffic_light.pose.pose.position.x = pos[0]
            traffic_light.pose.pose.position.y = pos[1]
            # TODO: figure the z value
            traffic_light.pose.pose.position.z = 5.83717
            self.traffic_light_info.lights.append(traffic_light)

    def process_traffic_lights(self):
        """Finds closest visible traffic light, if one exists, and determines its
            location and color

        Returns:
            int: index of waypoint closes to the upcoming traffic light (-1 if none exists)
            int: ID of traffic light color (specified in styx_msgs/TrafficLight)

        """
        light = None
        car_position_index = None
        closest_traffic_light_index = None

        if self.pose:
            car_position_index = self.get_closest_waypoint(self.pose.pose)

        # TODO find the closest visible traffic light (if one exists)
        if car_position_index is not None:
            closest_traffic_light_index = self.get_closest_traffic_light(car_position_index)

        if closest_traffic_light_index is not None:
            self.project_to_image_plane(self.traffic_light_info.lights[closest_traffic_light_index].pose.pose.position)

        if light:
            state = self.get_light_state(light)
            return light_wp, state
        # self.waypoints = None
        return -1, TrafficLight.UNKNOWN


if __name__ == '__main__':
    try:
        TLDetector()
    except rospy.ROSInterruptException:
        rospy.logerr('Could not start traffic node.')
