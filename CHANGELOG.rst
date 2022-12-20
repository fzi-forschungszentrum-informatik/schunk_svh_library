^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package schunk_svh_library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.0.1 (2022-12-20)
------------------
* Fix Errno 22 `invalid argument` for reading serial devices on ARM PCs
  This fixes an unsafe cast in `select`'s timeout parameter, which somehow
  only seemed to cause issues on a recent Raspberry Pi 4.
* Add CI for ROS2 Foxy and Humble
* Add license badge to the top-level readme
  That's easier for users to see.
* Fix workflows' file extension
* Merge pull request `#1 <https://github.com/fzi-forschungszentrum-informatik/schunk_svh_library/issues/1>`_ from fzi-forschungszentrum-informatik/add-github-ci
  Add GitHub ci
* Add build badges to top-level readme
* Activate CI for ROS Melodic and Noetic
* Merge branch 'fix-hardware-unit-test' into 'master'
  Outcomment hardware-dependend unit test
  See merge request hardware-drivers/schunk_svh_library!33
* Outcomment hardware-dependend unit test
* Merge branch 'update-maintainer' into 'master'
  Update maintainer
  See merge request hardware-drivers/schunk_svh_library!32
* Update maintainer
* Merge branch 'update-license' into 'master'
  Switch to GPLv3 license
  See merge request hardware-drivers/schunk_svh_library!31
* Fix typo in license notice
* Add license notice to all development files
  The text is in accordance with the recommendations from
  `here <https://www.gnu.org/licenses/gpl-howto.html>`_
  in the section *The license notices*.
* Add license notice for convenience
  Putting this language-specific summary in a separate folder is according
  to the ROS developers suggestions: http://wiki.ros.org/DevelopersGuide
* Update SPDX license indicator in package.xml
  This is according to
  `here <https://www.gnu.org/licenses/identify-licenses-clearly.html>`_.
* Add license text for the GPLv3
  The license text is from
  `here <https://www.gnu.org/licenses/gpl-3.0.txt>`_ after following the
  recommendations from `here <https://www.gnu.org/licenses/gpl-howto.html>`_.
* Added downstream workspaces
* Fixed library exports
* Export LIBRARIES and INCLUDE_DIRS
  This is important for legacy cmake systems such as catkin
* Use needs instead of dependencies
* Merge branch 'update-and-upgrade' into 'master'
  Make this a standalone library for driver development
  Closes `#11 <https://github.com/fzi-forschungszentrum-informatik/schunk_svh_library/issues/11>`_
  See merge request hardware-drivers/schunk_svh_library!25
* Contributors: Felix Exner, Stefan Scherzinger
