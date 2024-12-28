.. _roq-huobi:

.. |checkmark| unicode:: U+2713

roq-huobi
=========


Links
-----

* `Website <https://www.huobi.com/en-us/>`__
* `Support <https://huobiglobal.zendesk.com/hc/en-us>`__
* `API <https://huobiapi.github.io/docs/spot/v1/en/#change-log>`__


Supports
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto

      * - Spot
        - |checkmark|
      * - Futures
        -
      * - Options
        -
      * - Combos
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto

      * - Reference Data
        - |checkmark|
      * - Market Status
        - |checkmark|
      * - Top of Book
        - |checkmark|
      * - Market by Price
        - |checkmark|
      * - Market by Order
        -
      * - Trade Summary
        - |checkmark|
      * - Statistics
        - |checkmark|

  .. grid-item-card::  Order Management

    .. list-table::
      :widths: auto

      * - Create
        - |checkmark|
      * - Modify
        -
      * - Cancel
        - |checkmark|
      * - Cancel All
        - |checkmark|
      * - Auto-Cancel
        - |checkmark|

  .. grid-item-card::  Account Management

    .. list-table::
      :widths: auto

      * - Positions
        -
      * - Funds
        - |checkmark|


Installing
----------

* :ref:`Using Conda <tutorial-conda>`

.. tab:: Stable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/stable \
           roq-huobi

.. tab:: Unstable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/unstable \
           roq-huobi


Using
-----

.. code-block:: shell

   $ roq-huobi \
         --name "huobi" \
         --config_file $CONFIG_FILE_PATH \
         --client_listen_address $UNIX_SOCKET_PATH \
         --flagfile $ENVIRONMENT_FLAGFILE


.. _roq-huobi-flags:

Flags
-----

* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`

.. code-block:: shell

   $ roq-huobi --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: REST

   .. include:: flags/rest.rstinc

.. tab:: WS

   .. include:: flags/ws.rstinc

.. tab:: Misc

   .. include:: flags/misc.rstinc


Environments
------------

.. tab:: Prod

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-huobi/flags/prod/flags.cfg

   .. include:: flags/prod/flags.cfg
     :code: shell


Configuration
-------------

* :ref:`Gateway Config <gateway-config>`

.. code-block:: shell

   $ $CONDA_PREFIX/share/roq-huobi/config.toml

.. important::

   The template will be replaced when the software is upgraded.
   Make a copy and modify to your needs.

.. include:: config.toml
   :code: toml


Market Data
-----------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      -
      -
      -

    * - :cpp:class:`roq::MarketStatus`
      -
      -
      -

    * - :cpp:class:`roq::TopOfBook`
      - MarketData
      - market.$symbol.bbo
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      - MBPFeed
      - market.$symbol.mbp.20
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeSummary`
      - MarketData
      - market.$symbol.trade.detail
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      - MarketData
      - market.$symbol.ticker, market.$symbol.detail
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      - Rest
      - /v1/common/currencys, /v1/common/symbols
      -

    * - :cpp:class:`roq::MarketStatus`
      - MarketData
      -  /v1/common/symbols
      -

    * - :cpp:class:`roq::TopOfBook`
      -
      -
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      -
      -
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeSummary`
      -
      -
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      -
      -
      -


Statistics
~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`OPEN_PRICE`
    - (detail) :code:`open`

  * - :cpp:class:`HIGHEST_TRADED_PRICE`
    - (detail) :code:`high`

  * - :cpp:class:`LOWEST_TRADED_PRICE`
    - (detail) :code:`low`

  * - :cpp:class:`CLOSE_PRICE`
    - (detail) :code:`close`

  * - :cpp:class:`TRADE_VOLUME`
    - (detail) :code:`vol`


Order Management
----------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeUpdate`
      -
      -
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeUpdate`
      -
      -
      -

.. tab:: Request

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::CreateOrder`
      -
      -
      -

    * - :cpp:class:`roq::ModifyOrder`
      -
      -
      -

    * - :cpp:class:`roq::CancelOrder`
      -
      -
      -

    * - :cpp:class:`roq::CancelAllOrders`
      -
      -
      -

.. tab:: Response

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderAck`
      -
      -
      -


Order Types
~~~~~~~~~~~

TBD


Time in Force
~~~~~~~~~~~~~

TBD


Position Effect
~~~~~~~~~~~~~~~

TBD


Execution Instructions
~~~~~~~~~~~~~~~~~~~~~~

TBD


Account Management
------------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      -
      -
      -

    * - :cpp:class:`roq::FundsUpdate`
      -
      -
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      -
      -
      -

    * - :cpp:class:`roq::FundsUpdate`
      -
      -
      -


Streams
-------

.. tab:: Rest

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - REST
      - Primary purpose

.. tab:: MarketData

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - WebSocket
      - Primary purpose

.. tab:: MBPFeed

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - WebSocket
      - Primary purpose


Constraints
-----------


Comments
--------

* Only including symbols having :code:`state=ONLINE` and :code:`api_trading=ENABLED`
